vec3 ledMatrixEmission(vec3 source, float mode)
{
    source = clamp(source, 0.0, 1.0);
    if (mode < .5) return source;
    if (mode > 5.5) return vec3(source.r, source.g * .5, 0.0);
    float luminance = dot(source, vec3(.2126, .7152, .0722));
    if (mode < 1.5) return luminance * vec3(1.0, .03, .01);
    if (mode < 2.5) return luminance * vec3(.03, 1.0, .08);
    if (mode < 3.5) return luminance * vec3(1.0, .42, .02);
    if (mode < 4.5) return luminance * vec3(.03, .25, 1.0);
    return vec3(luminance);
}

vec3 matrixSource(vec2 uv)
{
    return adjustLinear(linearSourceSample(uv), Parameters.Adjustments);
}

vec3 sampleMatrixSource(vec2 index, vec2 cells)
{
    vec2 center = (index + .5) / cells;
    vec2 offset = .25 * step(vec2(.5), crtInputSize() - cells) / cells;
    vec3 source = matrixSource(center);
    if (any(greaterThan(offset, vec2(0.0))))
        source = (4.0 * source + matrixSource(center - offset) + matrixSource(center + offset)
            + matrixSource(center + vec2(offset.x, -offset.y))
            + matrixSource(center + vec2(-offset.x, offset.y))) * .125;
    return source;
}

float ledMatrixDistance(vec2 local, float shape)
{
    if (shape < .5) return length(local) - .36;
    float corner = shape >= 1.5 ? .10 : 0.0;
    vec2 q = abs(local) - vec2(.34 - corner);
    return length(max(q, vec2(0.0))) + min(max(q.x, q.y), 0.0) - corner;
}

vec3 ledMatrixPixel(vec2 uv)
{
    vec4 panel = Parameters.LedMatrix;
    vec2 cells = panel.xy;
    vec2 position = clamp(uv * cells, vec2(0.0), cells - .0001);
    vec2 index = floor(position);
    vec3 source = sampleMatrixSource(index, cells);

    vec3 light = ledMatrixEmission(source, panel.z);
    vec2 scale = Parameters.Output.xy / cells;
    float cellScale = min(scale.x, scale.y);
    float edge = clamp(.5 / cellScale, .015, .20);
    float distance = ledMatrixDistance((position - index - .5) * scale / cellScale, panel.w);
    float core = 1.0 - smoothstep(-edge, edge, distance);
    float energy = max(light.r, max(light.g, light.b));
    float halo = (1.0 - smoothstep(0.0, .05 + .09 * energy, max(distance, 0.0)))
        * (1.0 - core) * (.25 * energy);
    vec3 cell = clamp(vec3(.0045 + .010 * core) + light * (core + halo), 0.0, 1.0);
    vec3 color = mix(light, cell, smoothstep(1.0, 2.5, cellScale));
    return vec3(linearToSrgb(color.r), linearToSrgb(color.g), linearToSrgb(color.b));
}
