#version 450

layout(location = 0) out vec2 uv;

layout(std140, binding = 0) uniform SamplingParameters {
    vec4 Output;
    ivec4 Settings;
    vec4 Adjustments;
    vec4 Corrections;
} Parameters;

void main() {
    uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(uv.x * 2.0 - 1.0, (1.0 - uv.y * 2.0) * Parameters.Settings.y, 0.0, 1.0);
}
