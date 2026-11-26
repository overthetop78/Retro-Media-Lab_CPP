vec3 encodeSegmentColor(float level, float core, float halo, vec4 optics, float background)
{
    vec3 tint = optics.rgb;
    vec3 color;
    if (optics.w > .5 && optics.w < 1.5) {
        vec3 lcd = vec3(.57, .61, .48);
        color = mix(lcd, mix(lcd * .92, tint, level), core);
    } else {
        vec3 panel = optics.w > 1.5 ? vec3(.002, .0045, .0055) : vec3(background);
        color = min(panel + tint * (core * (.006 + .94 * level) + halo), vec3(1.0));
    }
    return vec3(linearToSrgb(color.r), linearToSrgb(color.g), linearToSrgb(color.b));
}

vec3 nativeSegmentPixel(vec2 uv)
{
    vec4 levels = Parameters.NativeSegmentLevels;
    vec4 optics = Parameters.NativeSegmentOptics;
    float red = sourcePoint(uv).r;
    float range = levels.y - levels.x;
    float level = clamp((red - levels.x) / range, 0.0, 1.0);
    float core = step(.5 / 255.0, red);
    float halo = 0.0;
    if (optics.w > 1.5) {
            vec2 spread = 1.35 / crtInputSize();
            float nearby = (sourcePoint(uv + vec2(spread.x, 0.0)).r +
                sourcePoint(uv - vec2(spread.x, 0.0)).r +
                sourcePoint(uv + vec2(0.0, spread.y)).r +
                sourcePoint(uv - vec2(0.0, spread.y)).r) * .25;
            float scattered = clamp((nearby - levels.x) / range, 0.0, 1.0);
            halo = scattered * scattered * (1.0 - core) * .14;
    }
    return encodeSegmentColor(level, core, halo, optics, .004);
}
