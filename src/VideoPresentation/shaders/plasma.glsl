const vec3 plasmaLuminanceWeights = vec3(.2126, .7152, .0722);

float plasmaBayer(vec2 cell, float phase)
{
    const float bayer[16] = float[16](0.0, 8.0, 2.0, 10.0, 12.0, 4.0, 14.0, 6.0,
        3.0, 11.0, 1.0, 9.0, 15.0, 7.0, 13.0, 5.0);
    ivec2 coordinate = ivec2(mod(cell + vec2(phase, floor(phase / 4.0)), 4.0));
    return bayer[coordinate.y * 4 + coordinate.x];
}

vec3 plasmaEmission(vec3 color, vec2 cell)
{
    vec4 emission = Parameters.PlasmaEmission;
    float averageLight = Parameters.PlasmaTiming.y;
    float luminance = dot(color, plasmaLuminanceWeights);
    vec3 saturated = mix(vec3(luminance), color, 1.0 + emission.x * .28);
    color = clamp(saturated + max(saturated - .42, 0.0) * emission.x * .48, 0.0, 1.0);
    color *= 1.0 - emission.y * .42 * smoothstep(.28, .78, averageLight);
    float levels = mix(255.0, 31.0, emission.z);
    float threshold = (plasmaBayer(cell, Parameters.PlasmaTiming.x) + .5) / 16.0;
    vec3 quantized = floor(color * levels + threshold) / levels;
    return mix(vec3(.0025), vec3(1.0), clamp(mix(color, quantized, emission.z), 0.0, 1.0));
}

vec3 plasmaCells(vec3 color, vec2 uv, vec2 panel, vec2 outputSize, int mode)
{
    vec2 pixel = clamp(floor(uv * outputSize), vec2(0.0), outputSize - 1.0);
    vec2 neutral = vec2(panelCellAxis(pixel.x, panel.x, outputSize.x, false, 0.0).x,
        panelCellAxis(pixel.y, panel.y, outputSize.y, false, 0.0).x);
    if (mode == 3) return dot(color, plasmaLuminanceWeights) * vec3(1.0, .29, .025) * neutral.x * neutral.y;
    if (mode == 2) {
        vec2 span = panel / outputSize;
        vec2 center = fract((pixel + .5) * span);
        float amount = smoothstep(2.0, 5.0, 1.0 / max(span.x, span.y));
        if (amount <= 0.0) return color * neutral.x * neutral.y;
        vec3 columns = vec3(panelCoverage(center.x, span.x, .04, .46),
            panelCoverage(center.x, span.x, .29, .71), panelCoverage(center.x, span.x, .54, .96));
        vec2 rows = vec2(panelCoverage(center.y, span.y, .54, .96), panelCoverage(center.y, span.y, .04, .46));
        return color * mix(vec3(neutral.x * neutral.y), columns * rows.xyx, amount);
    }
    vec3 columns = panelCellAxis(pixel.x, panel.x, outputSize.x, true, mode == 1 ? .018 : 0.0);
    return color * columns * (mode == 1 ? neutral.y : 1.0);
}

vec3 plasmaSource(vec2 texel, vec2 size, bool previous)
{
    vec3 rgb = displayHistoryTexel(texel, size, previous);
    return adjustDisplayHistoryColor(rgb);
}

vec3 plasmaPixel(vec2 uv)
{
    vec4 geometry = Parameters.PlasmaGeometry;
    vec2 outputSize = Parameters.Output.xy;
    vec2 cell = panelSampleCell(uv, geometry.xy, outputSize);
    vec2 size = vec2(geometry.w, crtInputSize().y);
    vec2 texel = floor((cell + .5) * size / geometry.xy);
    vec3 color = plasmaEmission(plasmaSource(texel, size, false), cell);
    if (crtInputSize().x > geometry.w) {
        vec3 previous = plasmaEmission(plasmaSource(texel, size, true), cell);
        color = max(color, previous * Parameters.PlasmaEmission.w);
    }
    color = plasmaCells(color, uv, geometry.xy, outputSize, int(geometry.z + .5));
    return vec3(linearToSrgb(color.r), linearToSrgb(color.g), linearToSrgb(color.b));
}
