#version 450
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 color;

layout(std140, binding = 0) uniform SamplingParameters {
    vec4 Output;
    ivec4 Settings;
    vec4 Adjustments;
    vec4 Corrections;
    vec4 CrtRaster;
    vec4 CrtTuning;
    vec4 CrtPhosphor;
    vec4 CrtGeometry;
    vec4 LedMatrix;
    vec4 VfdStructure;
    vec4 VfdPhosphor;
    vec4 DotMatrixGeometry;
    vec4 DotMatrixOptics;
    vec4 NativeSegmentLevels;
    vec4 NativeSegmentOptics;
    vec4 RasterSegmentGeometry;
    vec4 RasterSegmentShape;
    vec4 RasterSegmentLight;
    vec4 RasterSegmentTiming;
    vec4 RasterSegmentLines[19];
    vec4 EPaperInkAndColor;
    vec4 EPaperSurface;
    vec4 EPaperTemporal;
    vec4 ProjectionGeometry;
    vec4 ProjectionOptics;
    vec4 ProjectionScreen;
    vec4 PlasmaGeometry;
    vec4 PlasmaEmission;
    vec4 PlasmaTiming;
    vec4 VectorPhosphor;
    vec4 VectorBeam;
    vec4 VectorSource;
    vec4 FixedGrid;
    vec4 FixedOptics;
    vec4 FixedLight;
    vec4 FixedTiming;
} Parameters;

layout(binding = 1) uniform sampler2D PointSampler;

// The source texture has a single level. An explicit LOD also permits sampling in dynamic loops.
vec4 sourcePointRgba(vec2 coordinates) { return textureLod(PointSampler, coordinates, 0.0); }
vec3 sourcePoint(vec2 coordinates) { return sourcePointRgba(coordinates).rgb; }
vec3 sourceColor(vec2 coordinates) { return sourcePoint(coordinates); }
#include "image-adjustments.glsl"
#include "image-restoration.glsl"
vec2 crtInputSize() { return vec2(textureSize(PointSampler, 0)); }
#include "crt.glsl"
#include "led-matrix.glsl"
#include "vfd-cells.glsl"
#include "dot-matrix.glsl"
#include "native-segments.glsl"
#include "raster-segments.glsl"
#include "epaper.glsl"
#include "panel-grid.glsl"
#include "projection.glsl"
#include "plasma.glsl"
#include "vector-raster.glsl"
#include "fixed-pixel.glsl"
#include "display-filter.glsl"

void main()
{
    color = vec4(displayFilterPixel(uv), 1.0);
}
