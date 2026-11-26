vec3 dotMatrixPixel(vec2 uv)
{
    vec4 grid = Parameters.DotMatrixGeometry;
    vec4 optics = Parameters.DotMatrixOptics;
    vec2 cells = grid.xy;
    vec2 position = clamp(uv * cells, vec2(0.0), cells - .0001);
    vec2 index = floor(position);
    vec3 source = sampleMatrixSource(index, cells);
    bool rgb = grid.z > .5;
    float level = rgb ? max(source.r, max(source.g, source.b)) : dot(source, vec3(.2126, .7152, .0722));
    vec3 tint = rgb ? source / max(level, .0001) : optics.rgb;

    vec2 local = abs(position - index - .5);
    float distance = grid.w < .5 ? length(local) - .40
        : max(local.x - (grid.w < 1.5 ? .40 : .30), local.y - .40);
    vec2 scales = Parameters.Output.xy / cells;
    float scale = min(scales.x, scales.y);
    float edge = min(.5 / max(scale, 1.0), .20);
    float openArea = grid.w < .5 ? .503 : (grid.w < 1.5 ? .64 : .48);
    float coverage = mix(openArea, 1.0 - smoothstep(-edge, edge, distance), smoothstep(1.0, 2.5, scale));

    vec3 color;
    if (optics.w > 1.5) {
        color = mix(vec3(.004), vec3(.009) + .97 * tint * level, coverage);
    } else {
        bool backlit = optics.w > .5;
        vec3 background = backlit ? vec3(.70, .72, .69) : vec3(.462, .533, .258);
        vec3 inactive = background * (backlit ? .94 : .96);
        float transmission = backlit ? .085 : .042;
        if (rgb) {
            float chroma = (level - min(source.r, min(source.g, source.b))) / max(level, .0001);
            transmission = mix(transmission, backlit ? .60 : .42, chroma);
        }
        vec3 activatedDot = vec3(.014) + tint * transmission;
        color = mix(background, mix(inactive, activatedDot, level), coverage);
    }
    return vec3(linearToSrgb(color.r), linearToSrgb(color.g), linearToSrgb(color.b));
}
