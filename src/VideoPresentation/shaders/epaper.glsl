vec3 ePaperPigment(vec3 color, int mode, vec4 surface)
{
    color = clamp(color, vec3(0.0), vec3(1.0));
    const vec3 weights = vec3(.2126, .7152, .0722);
    float y = dot(color, weights);
    if (mode == 2 || mode == 5) {
        float levels = mode == 2 ? 15.0 : 31.0;
        vec3 ink = floor(color * levels + .5) / levels;
        ink = mix(vec3(dot(ink, weights)), ink, mode == 2 ? .56 : .94);
        return mix(vec3(surface.w), surface.rgb, ink);
    }
    if (mode == 3 && color.r > max(color.g, color.b) + .18 && color.r > .36)
        return vec3(.60, .031, .026);
    if (mode == 4 && min(color.r, color.g) > color.b + .17 && min(color.r, color.g) > .36)
        return vec3(.81, .65, .041);
    float level = mode == 1 ? floor(y * 15.0 + .5) / 15.0 : step(.48, y);
    return mix(vec3(surface.w), surface.rgb, level);
}

vec3 ePaperSource(vec2 pixel, vec2 size, bool previous)
{
    vec3 rgb = displayHistoryTexel(pixel, size, previous);
    return adjustDisplayHistoryColor(rgb);
}

vec3 ePaperPixel(vec2 uv)
{
    vec4 ink = Parameters.EPaperInkAndColor;
    vec4 surface = Parameters.EPaperSurface;
    vec4 refresh = Parameters.EPaperTemporal;
    vec2 cell = clamp(floor(uv * ink.xy), vec2(0.0), ink.xy - 1.0);
    vec2 size = vec2(ink.w, crtInputSize().y);
    vec2 pixel = floor((cell + .5) * size / ink.xy);
    int mode = int(ink.z + .5);
    vec3 current = ePaperPigment(ePaperSource(pixel, size, false), mode, surface);
    vec3 pigment = current;
    if (refresh.w > .5) {
        vec3 previous = ePaperPigment(ePaperSource(pixel, size, true), mode, surface);
        pigment = mix(previous, current, refresh.x);
        if (refresh.y > 0.0 && length(current - previous) > .01)
            pigment = mix(pigment, clamp(surface.rgb + surface.w - pigment, vec3(0.0), vec3(1.0)), refresh.y);
    }
    return vec3(linearToSrgb(pigment.r), linearToSrgb(pigment.g), linearToSrgb(pigment.b));
}
