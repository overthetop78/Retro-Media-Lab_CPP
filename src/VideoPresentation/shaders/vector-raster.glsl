vec3 vectorTexel(vec2 pixel, vec2 size, bool previous)
{
    vec3 color = displayHistoryTexel(pixel, size, previous);
    return vec3(srgbToLinear(color.r), srgbToLinear(color.g), srgbToLinear(color.b));
}

vec3 vectorSample(vec2 uv, bool previous)
{
    vec2 size = vec2(Parameters.VectorSource.x, crtInputSize().y);
    vec2 pixel = clamp(uv * size - .5, vec2(0.0), size - 1.0);
    vec2 base = floor(pixel);
    vec2 weight = fract(pixel);
    vec3 top = mix(vectorTexel(base, size, previous), vectorTexel(base + vec2(1.0, 0.0), size, previous), weight.x);
    vec3 bottom = mix(vectorTexel(base + vec2(0.0, 1.0), size, previous),
        vectorTexel(base + vec2(1.0), size, previous), weight.x);
    return mix(top, bottom, weight.y);
}

vec3 vectorEmission(vec2 uv, bool previous)
{
    const vec3 luminance = vec3(.2126, .7152, .0722);
    vec4 beam = Parameters.VectorBeam;
    float gain = Parameters.VectorSource.y;
    vec2 radius = beam.x / Parameters.Output.xy;
    vec3 center = vectorSample(uv, previous);
    vec3 neighbors = .25 * (vectorSample(uv - vec2(radius.x, 0.0), previous) +
        vectorSample(uv + vec2(radius.x, 0.0), previous) +
        vectorSample(uv - vec2(0.0, radius.y), previous) +
        vectorSample(uv + vec2(0.0, radius.y), previous));
    float attenuation = 1.0 - max(0.0, 1.0 - beam.y) * (1.0 - dot(center, luminance));
    float light = clamp(dot(neighbors, luminance) * gain, 0.0, 1.0);
    vec4 phosphor = Parameters.VectorPhosphor;
    float diffusion = phosphor.w * sqrt(beam.y) * light;
    vec3 color = mix(center, neighbors, min(.72, .18 * beam.y)) * attenuation + neighbors * diffusion;
    if (any(greaterThan(phosphor.rgb, vec3(0.0)))) color = dot(color, luminance) * phosphor.rgb;
    return color * gain;
}

vec3 vectorAdjustedPixel(vec2 uv)
{
    vec3 color = vectorEmission(uv, false);
    float decay = Parameters.VectorBeam.w;
    if (decay > .001) color += vectorEmission(uv, true) * decay;
    return adjustLinear(color, Parameters.Adjustments);
}

vec3 vectorRasterPixel(vec2 uv)
{
    uv = crtScreenUv(uv, Parameters.VectorBeam.z);
    if (!crtOnScreen(uv)) return vec3(0.0);
    vec3 color = vectorAdjustedPixel(uv);
    float sharpness = Parameters.Corrections.x;
    if (abs(sharpness) > .0001) {
        vec2 d = 1.0 / Parameters.Output.xy;
        vec3 sum = color;
        float weight = 1.0;
        const vec2 offsets[12] = vec2[12](
            vec2(-1, -1), vec2(0, -1), vec2(1, -1), vec2(-1, 0),
            vec2(1, 0), vec2(-1, 1), vec2(0, 1), vec2(1, 1),
            vec2(-2, 0), vec2(2, 0), vec2(0, -2), vec2(0, 2));
        for (int i = 0; i < int(Parameters.VectorSource.z); ++i) {
            float tapWeight = i < 8 ? 1.0 : .5;
            sum += tapWeight * vectorAdjustedPixel(uv + offsets[i] * d);
            weight += tapWeight;
        }
        color = videoSharpnessParameter(color, sum / weight, sharpness);
    }
    return vec3(linearToSrgb(color.r), linearToSrgb(color.g), linearToSrgb(color.b));
}
