vec3 vfdPixel(vec2 uv)
{
    vec4 panel = Parameters.VfdStructure;
    vec2 cells = panel.xy;
    vec2 position = clamp(uv * cells, vec2(0.0), cells - .0001);
    vec2 index = floor(position);
    float level = dot(sampleMatrixSource(index, cells), vec3(.2126, .7152, .0722));
    vec2 local = abs(position - index - .5);
    float shape = panel.z;
    float distance = shape < .5 ? length(local) - .38
        : max(local.x - (shape < 1.5 ? .38 : .28), local.y - .38);
    vec2 scales = Parameters.Output.xy / cells;
    float scale = min(scales.x, scales.y);
    float edge = clamp(.5 / max(scale, 1.0), .025, .20);
    float core = 1.0 - smoothstep(-edge, edge, distance);
    float meanArea = shape < .5 ? .454 : (shape < 1.5 ? .578 : .426);
    float detail = smoothstep(1.0, 2.5, scale);
    float coverage = mix(meanArea, core, detail);
    float glowRadius = mix(.035, .115, level);
    float glow = (1.0 - smoothstep(0.0, glowRadius, max(distance, 0.0)))
        * (1.0 - core) * (.24 * level * level) * detail;
    vec3 phosphor = Parameters.VfdPhosphor.rgb;
    vec3 glass = vec3(.002, .0045, .0055);
    vec3 darkCell = vec3(.0018) + phosphor * .004;
    vec3 color = min(glass + darkCell * coverage + phosphor * (.94 * level * coverage + glow), vec3(1.0));
    return vec3(linearToSrgb(color.r), linearToSrgb(color.g), linearToSrgb(color.b));
}
