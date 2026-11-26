// CRT signal and phosphor mask from Retro Media Lab C# (osd-support).
vec2 crtScreenUv(vec2 uv, float curvature)
{
    vec2 offset = uv - vec2(.5);
    return uv + offset * (4.0 * curvature * dot(offset, offset));
}
vec2 crtScreenFootprint(vec2 uv, vec2 pixelSize, float curvature)
{
    vec2 offset = uv - vec2(.5);
    float radialScale = 1.0 + 4.0 * curvature * dot(offset, offset);
    vec2 diagonal = vec2(radialScale) + 8.0 * curvature * offset * offset;
    float crossTerm = 8.0 * curvature * abs(offset.x * offset.y);
    return diagonal * pixelSize + crossTerm * pixelSize.yx;
}
bool crtOnScreen(vec2 uv)
{
    return all(greaterThanEqual(uv, vec2(0.0))) && all(lessThanEqual(uv, vec2(1.0)));
}
vec2 crtSignalUv(vec2 uv, vec4 raster, vec4 tuning)
{
    if (tuning.x > 0.0)
    {
        float receiverTime = .5 + (uv.y - .5) * raster.z;
        float sourceTime = fract((receiverTime - .5) * tuning.x + .5 + raster.w);
        float sourceAperture = tuning.x > 1.0 ? 480.0 / 525.0 : 576.0 / 625.0;
        if (abs(sourceTime - .5) > sourceAperture * .5) return vec2(-1.0);
        float sourcePhase = (sourceTime - .5) / sourceAperture;
        float linesFromRetrace = sourceTime * raster.y / raster.z;
        uv.x -= .20 * sin(6.2831853 * (linesFromRetrace * tuning.z + raster.w));
        uv.y = sourcePhase * raster.x + .5;
        return uv;
    }
    return uv;
}
vec2 crtChromaNeighbor(vec2 uv, vec2 sourceSize, int decoder)
{
    return decoder == 1 ? uv - vec2(1.0 / sourceSize.x, 0.0)
        : uv - vec2(0.0, 1.0 / sourceSize.y);
}
vec3 crtDecode(vec3 center, vec3 neighbor, int decoder, float line)
{
    const vec3 weights = vec3(.2126,.7152,.0722);
    float luma = dot(center, weights);
    if (decoder == 0) return vec3(luma);
    vec3 chroma = center - vec3(luma);
    vec3 neighborChroma = neighbor - vec3(dot(neighbor, weights));
    if (decoder == 1) chroma = mix(chroma, neighborChroma, .45);
    else if (decoder == 2) chroma = mix(chroma, neighborChroma, .25);
    else
    {
        if (mod(line, 2.0) < .5) chroma.b = neighborChroma.b;
        else chroma.r = neighborChroma.r;
        chroma.g = -(weights.r * chroma.r + weights.b * chroma.b) / weights.g;
    }
    return clamp(vec3(luma) + chroma, 0.0, 1.0);
}
float crtBeam(float y, float height, vec4 raster, float exponent, float mask)
{
    if (int(mask + .5) == 1) return 1.0;
    float strength = .18 * clamp(height / raster.y - 1.0, 0.0, 1.0);
    float wave = .5 + .5 * cos(6.2831853 * (y * raster.y - .5));
    return 1.0 - strength * (1.0 - pow(clamp(wave, 0.0, 1.0), exponent));
}

float crtOpeningIntegral(float x, float period, float length)
{
    float cycles = floor(x / period);
    return cycles * length + clamp(x - cycles * period, 0.0, length);
}
float crtOpening(float position, float footprint, float period, float center, float radius)
{
    float start = position - center + radius;
    float length = 2.0 * radius;
    return (crtOpeningIntegral(start + footprint * .5, period, length)
        - crtOpeningIntegral(start - footprint * .5, period, length)) / footprint;
}
float crtDotOpening(float position, float footprint, float period, float center, float radius)
{
    return .65 * crtOpening(position, footprint, period, center, radius)
        + .35 * crtOpening(position, footprint, period, center, radius * .7);
}
float crtDot(vec2 position, vec2 footprint, vec2 pitch, vec2 center, vec2 radius)
{
    return crtDotOpening(position.x, footprint.x, pitch.x, center.x, radius.x)
            * crtDotOpening(position.y, footprint.y, 2.0 * pitch.y, center.y, radius.y)
        + crtDotOpening(position.x, footprint.x, pitch.x, center.x + .5 * pitch.x, radius.x)
            * crtDotOpening(position.y, footprint.y, 2.0 * pitch.y, center.y + pitch.y, radius.y);
}
vec3 crtStripeIntegral(float x, float width, float period, vec3 offsets)
{
    float periods = floor(x / period);
    float phase = x - periods * period;
    return vec3(periods * width) + clamp(vec3(phase) - offsets, 0.0, width);
}
vec3 crtStripeCoverage(float x, float footprint, float width, float period, float spacing)
{
    vec3 offsets = vec3(0.0, spacing, 2.0 * spacing);
    return (crtStripeIntegral(x + footprint * .5, width, period, offsets)
        - crtStripeIntegral(x - footprint * .5, width, period, offsets)) / footprint;
}
vec3 crtMaskCoverage(vec2 position, int mask, vec2 footprint)
{
    if (mask == 1) return crtStripeCoverage(position.x, footprint.x, 1.75, 6.0, 2.0);
    if (mask == 3) return crtStripeCoverage(position.x, footprint.x, 1.75, 6.0, 1.75);
    if (mask == 2 || mask == 6)
    {
        vec2 pitch = mask == 6 ? vec2(6.0, 7.0) : vec2(8.0, 6.0);
        vec2 radius = mask == 6 ? vec2(1.4, 2.55) : vec2(1.85);
        return vec3(
            crtDot(position, footprint, pitch, vec2(pitch.x * .25, 2.0), radius),
            crtDot(position, footprint, pitch, vec2(pitch.x * .75, 2.0), radius),
            crtDot(position, footprint, pitch, vec2(pitch.x * .50, pitch.y - 1.5), radius));
    }
    return crtOpening(position.y, footprint.y, 10.0, 5.0, 4.7)
            * crtStripeCoverage(position.x, footprint.x, 1.85, 12.0, 1.85)
        + crtOpening(position.y, footprint.y, 10.0, 0.0, 4.7)
            * crtStripeCoverage(position.x - 6.0, footprint.x, 1.85, 12.0, 1.85);
}
vec3 crtEmit(vec3 color, vec2 uv, vec2 grid, vec4 phosphor, vec2 footprint)
{
    int mask = int(phosphor.w + .5);
    if (mask == 5) return dot(color, vec3(.2126,.7152,.0722)) * phosphor.xyz;
    return color * crtMaskCoverage(uv * grid, mask, footprint);
}
vec3 crtImageAt(vec2 uv);
vec3 crtNeighborEmission(vec2 uv, vec2 grid, vec4 phosphor, vec2 footprint)
{
    if (any(lessThan(uv, vec2(0.0))) || any(greaterThanEqual(uv, vec2(1.0)))) return vec3(0.0);
    return crtEmit(crtImageAt(uv), uv, grid, phosphor, footprint);
}
vec3 crtCompose(vec3 color, vec2 uv, vec2 rasterSize, vec4 phosphor, float exponent, float gain,
    vec2 outputSize, vec2 viewUv, float curvature)
{
    int mask = int(phosphor.w + .5);
    if (mask == 0) return color;
    vec2 grid = rasterSize * 3.0;
    vec2 footprint = grid / max(vec2(1.0), outputSize);
    if (curvature > 0.0)
        footprint = grid * crtScreenFootprint(viewUv, 1.0 / max(vec2(1.0), outputSize), curvature);
    vec2 across = vec2(1.0 / grid.x, 0.0);
    vec2 down = vec2(0.0, 1.0 / grid.y);
    vec3 direct = crtEmit(color, uv, grid, phosphor, footprint);
    vec3 adjacent = (crtNeighborEmission(uv - across, grid, phosphor, footprint)
        + crtNeighborEmission(uv + across, grid, phosphor, footprint)
        + crtNeighborEmission(uv - down, grid, phosphor, footprint)
        + crtNeighborEmission(uv + down, grid, phosphor, footprint)) * .25;
    float spread = clamp(.72 * sqrt(gain) / (1.0 + exponent * exponent), 0.0, .78);
    float glass = 0.0;
    if (mask != 5)
    {
        vec2 position = uv * grid;
        vec3 coverage = crtMaskCoverage(position, mask, footprint);
        glass = .00010 + .00075 * (coverage.r + coverage.g + coverage.b);
    }
    return mix(direct, adjacent, spread) + vec3(glass);
}

vec3 crtAdjustedPoint(vec2 coordinates)
{
    vec3 color = adjustedLinearSample(coordinates, Parameters.Adjustments);
    return sharpenAdjustedSample(color, coordinates, Parameters.Output.xy, Parameters.Adjustments, Parameters.Corrections.x);
}

vec3 crtImageAt(vec2 coordinates)
{
    vec2 source = crtSignalUv(coordinates, Parameters.CrtRaster, Parameters.CrtTuning);
    if (!crtOnScreen(source)) return vec3(0.0);
    vec2 size = crtInputSize();
    vec3 center = crtAdjustedPoint(source);
    int decoder = int(Parameters.CrtTuning.y + .5);
    if (decoder < 4) {
        vec3 neighbor = decoder > 0
            ? crtAdjustedPoint(crtChromaNeighbor(source, size, decoder)) : center;
        center = crtDecode(center, neighbor, decoder, floor(source.y * size.y));
    }
    float gain = Parameters.CrtGeometry.z;
    return min(vec3(1.0), center * (gain * crtBeam(coordinates.y, Parameters.Output.y,
        Parameters.CrtRaster, Parameters.CrtGeometry.y, Parameters.CrtPhosphor.w)));
}

vec3 crtPixel(vec2 viewUv)
{
    float curvature = Parameters.CrtGeometry.x;
    vec2 screen = crtScreenUv(viewUv, curvature);
    if (!crtOnScreen(screen)) return vec3(0.0);

    vec3 color = crtCompose(crtImageAt(screen), screen, vec2(Parameters.CrtTuning.w, 2.0 * Parameters.CrtRaster.y),
        Parameters.CrtPhosphor, Parameters.CrtGeometry.y, Parameters.CrtGeometry.z, Parameters.Output.xy, viewUv, curvature);
    return vec3(linearToSrgb(color.r), linearToSrgb(color.g), linearToSrgb(color.b));
}
