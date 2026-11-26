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
} Parameters;

layout(binding = 1) uniform sampler2D PointSampler;

vec3 sourcePoint(vec2 coordinates) { return texture(PointSampler, coordinates).rgb; }
vec3 sourceColor(vec2 coordinates) { return sourcePoint(coordinates); }
#include "image-adjustments.glsl"
#include "image-restoration.glsl"
vec2 crtInputSize() { return vec2(textureSize(PointSampler, 0)); }
#include "crt.glsl"

void main()
{
    color = vec4(crtPixel(uv), 1.0);
}
