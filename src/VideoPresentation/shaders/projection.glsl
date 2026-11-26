float projectionAperture(float position, float cells, float pixels, float border)
{
    if (border <= 0.0 || pixels <= cells) return 1.0;
    float span = cells / pixels;
    float center = fract((position + .5) * span);
    return mix(1.0, panelCoverage(center, span, border, 1.0 - border), smoothstep(1.0, 2.5, pixels / cells));
}

vec3 projectionSource(vec2 uv)
{
    vec2 outputSize = Parameters.Output.xy;
    vec2 panel = Parameters.ProjectionGeometry.xy;
    vec2 center = (panelSampleCell(uv, panel, outputSize) + .5) / panel;
    vec2 size = crtInputSize();
    return adjustDisplayHistoryColor(panelDecodedTexel(floor(center * size), size, false, Parameters.ProjectionGeometry.z));
}

vec3 projectionPixel(vec2 uv)
{
    vec2 outputSize = Parameters.Output.xy;
    vec2 panel = Parameters.ProjectionGeometry.xy;
    vec4 optics = Parameters.ProjectionOptics;
    vec3 screen = Parameters.ProjectionScreen.xyz;
    vec2 pixel = clamp(floor(uv * outputSize), vec2(0.0), outputSize - 1.0);
    vec2 spread = max(1.0 / outputSize, .55 / panel);
    float sourcePixel = 1.0 / crtInputSize().x;
    const int OpticalTaps = 7;
    vec2 offsets[OpticalTaps] = vec2[OpticalTaps](vec2(0.0), vec2(-spread.x, 0.0), vec2(spread.x, 0.0),
        vec2(0.0, -spread.y), vec2(0.0, spread.y), vec2(-sourcePixel, 0.0), vec2(sourcePixel, 0.0));
    vec3 color = vec3(0.0);
    vec3 surround = vec3(0.0);
    int count = optics.y > 0.0 ? OpticalTaps : 1;
    for (int i = 0; i < count; ++i) {
        vec3 sampleColor = projectionSource(uv + offsets[i]);
        if (i == 0) color = sampleColor;
        else if (i < 5) surround += sampleColor;
        else if (i == 5) color.r = mix(color.r, sampleColor.r, optics.w);
        else color.b = mix(color.b, sampleColor.b, optics.w);
    }
    color = mix(color, surround * .25, optics.y);
    float area = projectionAperture(pixel.x, panel.x, outputSize.x, optics.z) *
        projectionAperture(pixel.y, panel.y, outputSize.y, optics.z);
    color = mix(vec3(optics.x), vec3(1.0), color * area);
    vec2 p = pixel + .5;
    float textureLevel = .5 + .25 * sin(p.x * .71 + p.y * .31) + .25 * sin(p.x * .17 - p.y * .83);
    color = mix(color, vec3(1.0), screen.y) * screen.x * (1.0 - screen.z * textureLevel);
    return vec3(linearToSrgb(color.r), linearToSrgb(color.g), linearToSrgb(color.b));
}
