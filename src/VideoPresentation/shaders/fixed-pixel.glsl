const vec3 fixedLuminance = vec3(.2126, .7152, .0722);

vec3 lcdStructure(vec3 color, vec2 pixel, vec2 panel, vec2 outputSize, int mode, bool horizontal)
{
    if (mode == 0) return color;
    bool mono = mode == 3;
    vec3 columns = panelCellAxis(pixel.x, panel.x, outputSize.x, !mono && !horizontal, 0.0);
    vec3 rows = panelCellAxis(pixel.y, panel.y, outputSize.y, !mono && horizontal, 0.0);
    vec3 aperture = columns * rows;
    if (mono) color = vec3(dot(color, fixedLuminance));
    return color * (mode == 2 ? aperture.bgr : aperture);
}

float oledOpening(float center, float footprint, float first, float last)
{
    return clamp((min(center + footprint * .5, last) - max(center - footprint * .5, first)) / footprint, 0.0, 1.0);
}

float oledDiamond(vec2 position, vec2 footprint, vec2 center, vec2 radius)
{
    vec2 offset = abs(position - center) / radius;
    vec2 width = footprint / radius;
    return clamp(.5 + (1.0 - offset.x - offset.y) / max(width.x, width.y), 0.0, 1.0);
}

vec3 oledStructure(vec3 color, vec2 pixel, vec2 panel, vec2 outputSize, int mode)
{
    if (mode <= 2 || mode == 6)
        return lcdStructure(color, pixel, panel, outputSize, mode == 6 ? 3 : mode, false);

    vec2 scale = outputSize / panel;
    vec2 span = 1.0 / scale;
    vec2 cell = (pixel + .5) * span;
    vec2 position = fract(cell);
    vec2 neutral = vec2(panelCellAxis(pixel.x, panel.x, outputSize.x, false, 0.0).x,
        panelCellAxis(pixel.y, panel.y, outputSize.y, false, 0.0).x);
    float detail = mode == 3 ? smoothstep(3.5, 6.0, scale.x) : smoothstep(2.0, 4.0, min(scale.x, scale.y));
    if (detail <= 0.0) return color * neutral.x * neutral.y;
    vec3 emission;
    if (mode == 3) {
        float white = min(color.r, min(color.g, color.b)) * .60;
        vec3 rgb = vec3(oledOpening(position.x, span.x, .265, .500),
            oledOpening(position.x, span.x, .500, .735), oledOpening(position.x, span.x, .735, .970));
        emission = ((color - white) * rgb + white * oledOpening(position.x, span.x, .030, .265)) *
            oledOpening(position.y, span.y, .030, .970);
    } else if (mode == 4) {
        float lower = oledOpening(position.y, span.y, .56, .92);
        emission = color * vec3(lower * oledOpening(position.x, span.x, .03, .47),
            oledOpening(position.y, span.y, .08, .44) * oledOpening(position.x, span.x, .28, .72),
            lower * oledOpening(position.x, span.x, .53, .97));
    } else {
        float green = oledDiamond(position, span, vec2(.23, .5), vec2(.20, .36));
        float chroma = oledDiamond(position, span, vec2(.69, .5), vec2(.30, .48));
        bool red = mod(floor(cell.x) + floor(cell.y), 2.0) < .5;
        emission = color * vec3(red ? chroma : 0.0, green, red ? 0.0 : chroma);
    }
    return mix(color * neutral.x * neutral.y, emission, detail);
}

float lcdBand(float distance)
{
    return smoothstep(0.0, 1.0, 1.0 - abs(distance));
}

vec3 lcdLight(vec3 color, vec2 uv, vec2 outputSize, int technology, float zone)
{
    vec4 optics = Parameters.FixedOptics;
    vec4 settings = Parameters.FixedLight;
    float black = settings.y;
    float dark = clamp(1.0 - dot(color, fixedLuminance) / .22, 0.0, 1.0);
    float intensity = pow(settings.x, .8);
    float light = 1.0;
    if (technology == 1) {
        int backlight = int(optics.w + .5);
        float edge = exp(-9.0 * min(uv.y, 1.0 - uv.y)) * (.70 + .30 * sin(uv.x * 11.0 + uv.y * 2.0));
        if (backlight == 0) {
            light = .92 + .08 * edge;
            black *= 1.0 + 1.25 * edge * dark * dark;
        } else if (backlight == 1) {
            light = .98 + .02 * (1.0 - abs(2.0 * uv.y - 1.0));
        } else {
            light = zone;
        }
    } else {
        float top = .20 + .55 * lcdBand((uv.x - .28) * 2.8) + .20 * lcdBand((uv.x - .87) * 4.5);
        float bottom = .12 + .65 * lcdBand((uv.x - .63) * 2.5) + .27 * lcdBand((uv.x - .10) * 3.0);
        float spread = max(1.0, outputSize.y * .055);
        float t = uv.y * outputSize.y / spread;
        float b = (1.0 - uv.y) * outputSize.y / spread;
        float edge = .76 * top * mix(exp(-3.6 * t), exp(-2.0 * t), top) +
            bottom * mix(exp(-3.8 * b), exp(-2.3 * b), bottom);
        black += settings.z * edge * dark * dark;
    }
    return clamp(intensity * light * (vec3(black) + color * (1.0 - black)), 0.0, 1.0);
}

float fixedPeak(vec3 color)
{
    return max(color.r, max(color.g, color.b));
}

vec3 fixedPixel(vec2 uv)
{
    vec4 grid = Parameters.FixedGrid;
    vec4 optics = Parameters.FixedOptics;
    vec2 outputSize = Parameters.Output.xy;
    vec2 pixel = clamp(floor(uv * outputSize), vec2(0.0), outputSize - 1.0);
    vec2 size = vec2(grid.w, crtInputSize().y);
    vec2 texel = floor((panelSampleCell(uv, grid.xy, outputSize) + .5) * size / grid.xy);
    int technology = int(grid.z + .5);
    int structure = int(optics.x + .5);
    vec3 result;
    if (technology == 2) {
        vec3 color = adjustDisplayHistoryColor(displayHistoryTexel(texel, size, false));
        if (int(optics.y + .5) == 1) {
            float white = min(color.r, min(color.g, color.b));
            float desaturation = .25 * smoothstep(.45, 1.0, fixedPeak(color)) * smoothstep(0.0, .50, white);
            color = mix(color, vec3(dot(color, fixedLuminance)), desaturation);
        }
        color = clamp(color * (1.0 - Parameters.FixedLight.x * smoothstep(.18, .90, optics.w)), 0.0, 1.0);
        result = oledStructure(color, pixel, grid.xy, outputSize, structure);
    } else {
        float zone = 1.0;
        if (technology == 1 && optics.w >= 2.0)
            zone = sourcePointRgba((clamp(floor(uv * size), vec2(0.0), size - 1.0) + .5) / crtInputSize()).a;
        float elapsed = Parameters.FixedTiming.x;
        vec3 samples[2];
        int count = elapsed > 0.0 ? 2 : 1;
        for (int i = 0; i < count; ++i) {
            vec3 color = adjustDisplayHistoryColor(panelDecodedTexel(texel, size, i != 0, Parameters.FixedTiming.y));
            color = lcdStructure(color, pixel, grid.xy, outputSize, structure, optics.z > .5);
            samples[i] = lcdLight(color, (pixel + .5) / outputSize, outputSize, technology, zone);
        }
        result = samples[0];
        if (count == 2) {
            float response = Parameters.FixedLight.w;
            if (int(optics.y + .5) == 2) {
                float peak = min(fixedPeak(samples[1]), fixedPeak(result));
                response *= 1.0 + 2.2 * (1.0 - clamp(peak / .2, 0.0, 1.0));
            }
            result = mix(samples[1], result, 1.0 - exp(-max(.001, elapsed) / response));
        }
    }
    return vec3(linearToSrgb(result.r), linearToSrgb(result.g), linearToSrgb(result.b));
}
