vec3 adjustDisplayHistoryColor(vec3 color)
{
    return adjustLinear(vec3(srgbToLinear(color.r), srgbToLinear(color.g), srgbToLinear(color.b)), Parameters.Adjustments);
}

vec3 displayHistoryTexel(vec2 pixel, vec2 size, bool previous)
{
    vec2 offset = previous ? vec2(size.x, 0.0) : vec2(0.0);
    return sourcePoint((clamp(pixel, vec2(0.0), size - 1.0) + offset + .5) / crtInputSize());
}

vec3 rasterSegmentSample(vec2 pixel, vec2 size, bool previous)
{
    vec3 color;
    if (previous) {
        vec2 base = floor(pixel - .5);
        vec2 fraction = fract(pixel - .5);
        vec3 top = mix(displayHistoryTexel(base, size, true),
            displayHistoryTexel(base + vec2(1.0, 0.0), size, true), fraction.x);
        vec3 bottom = mix(displayHistoryTexel(base + vec2(0.0, 1.0), size, true),
            displayHistoryTexel(base + vec2(1.0), size, true), fraction.x);
        color = mix(top, bottom, fraction.y);
    } else {
        color = displayHistoryTexel(floor(pixel), size, false);
    }
    return adjustDisplayHistoryColor(color);
}

float rasterSegmentActivation(vec2 cell, vec4 line, vec2 size, bool previous)
{
    const int samples = 5;
    int count = all(equal(line.xy, line.zw)) ? 1 : samples;
    float sum = 0.0;
    for (int i = 0; i < count; ++i) {
        float t = (float(i) + .5) / float(count);
        vec2 pixel = (cell + mix(line.xy, line.zw, t)) * Parameters.RasterSegmentGeometry.xy;
        sum += dot(rasterSegmentSample(pixel, size, previous), vec3(.2126, .7152, .0722));
    }
    vec2 light = Parameters.RasterSegmentLight.xy;
    float shifted = clamp(sum / float(count) - light.x + .5, 0.0, 1.0);
    return clamp((shifted - .5) * light.y + .5, 0.0, 1.0);
}

float rasterSegmentDistance(vec2 position, vec4 line, float shape)
{
    vec2 delta = position - line.xy;
    vec2 direction = line.zw - line.xy;
    float lengthSquared = dot(direction, direction);
    if (lengthSquared == 0.0) return length(delta);
    float along = dot(delta, direction) / lengthSquared;
    if (shape > .5 && shape < 1.5) return length(delta - direction * clamp(along, 0.0, 1.0));
    float side = length(delta - direction * along);
    float extension = max(0.0, abs(along - .5) - .5) * sqrt(lengthSquared);
    return max(side, shape > 1.5 ? extension : (extension + side) * .70710678);
}

vec3 rasterSegmentPixel(vec2 uv)
{
    vec4 geometry = Parameters.RasterSegmentGeometry;
    vec4 shape = Parameters.RasterSegmentShape;
    vec4 timing = Parameters.RasterSegmentTiming;
    vec2 position = uv * timing.xy / geometry.xy;
    vec2 local = (fract(position) - .5) / geometry.zw + .5;
    float nearest = 1.0e6;
    vec4 line = Parameters.RasterSegmentLines[0];
    for (int i = 0; i < int(shape.z); ++i) {
        vec4 candidate = Parameters.RasterSegmentLines[i];
        float distanceToLine = rasterSegmentDistance(local, candidate, shape.y);
        if (distanceToLine < nearest) {
            nearest = distanceToLine;
            line = candidate;
        }
    }
    vec2 cell = floor(position);
    float level = rasterSegmentActivation(cell, line, timing.xy, false);
    if (crtInputSize().x > timing.x) {
        float previous = rasterSegmentActivation(cell, line, timing.xy, true);
        level = level >= previous ? mix(previous, level, timing.z) : max(level, previous * timing.w);
    }
    vec2 scale = timing.xy / max(Parameters.Output.xy, vec2(1.0));
    float edge = max(scale.x, scale.y) / min(geometry.x, geometry.y);
    float core = 1.0 - smoothstep(shape.x - edge, shape.x + edge, nearest);
    vec2 haloSettings = Parameters.RasterSegmentLight.zw;
    float halo = exp(-max(0.0, nearest - shape.x) * haloSettings.x) * haloSettings.y * level * level * (1.0 - core);
    return encodeSegmentColor(level, core, halo, Parameters.NativeSegmentOptics, shape.w);
}
