vec2 panelSampleCell(vec2 uv, vec2 panel, vec2 outputSize)
{
    vec2 pixel = clamp(floor(uv * outputSize), vec2(0.0), outputSize - 1.0);
    return floor((pixel + .5) * panel / outputSize);
}

vec3 panelDecodedTexel(vec2 pixel, vec2 size, bool history, float decoderSetting)
{
    vec3 center = displayHistoryTexel(pixel, size, history);
    int decoder = int(decoderSetting) - 1;
    if (decoder < 0) return center;
    vec2 delta = decoder == 1 ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec3 neighbor = decoder == 0 ? center : displayHistoryTexel(pixel - delta, size, history);
    return floor(crtDecode(center, neighbor, decoder, pixel.y) * 255.0 + .5) / 255.0;
}

float panelOpenArea(float position, float start, float end)
{
    float width = end - start;
    return floor(position) * width + clamp(fract(position) - start, 0.0, width);
}

float panelCoverage(float center, float span, float start, float end)
{
    float covered = panelOpenArea(center + span * .5, start, end) -
        panelOpenArea(center - span * .5, start, end);
    return clamp(covered / span, 0.0, 1.0);
}

vec3 panelCellAxis(float pixel, float cells, float pixels, bool striped, float gap)
{
    if (pixels <= cells) return vec3(1.0);

    const float inset = .03;
    const float openWidth = 1.0 - 2.0 * inset;
    const float stripe = openWidth / 3.0;
    float span = cells / pixels;
    float scale = pixels / cells;
    float center = fract((pixel + .5) * span);
    float aperture = mix(openWidth, panelCoverage(center, span, inset, 1.0 - inset), smoothstep(1.0, 2.0, scale));
    float resolved = striped && scale >= 2.0 ? .08 + .92 * smoothstep(3.0, 5.0, scale) : 0.0;
    if (resolved <= 0.0) return vec3(aperture);

    float cell = floor((pixel + .5) * span);
    float start = ceil(cell * scale - .5);
    float end = ceil((cell + 1.0) * scale - .5);
    span = 1.0 / max(1.0, end - start);
    center = (pixel - start + .5) * span;
    vec3 openings = vec3(panelCoverage(center, span, inset + gap, inset + stripe - gap),
        panelCoverage(center, span, inset + stripe + gap, inset + 2.0 * stripe - gap),
        panelCoverage(center, span, inset + 2.0 * stripe + gap, 1.0 - inset - gap));
    return mix(vec3(aperture), openings, resolved);
}
