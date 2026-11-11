#version 450

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;

layout(push_constant) uniform Push {
    vec2 screenSize;
} pc;

layout(location = 0) out vec2 vUV;
layout(location = 1) out vec4 vColor;

void main() {
    vec2 ndc = inPos / pc.screenSize * 2.0 - 1.0;
    // En Vulkan el eje Y de NDC ya apunta hacia abajo (origen arriba-izquierda)
    gl_Position = vec4(ndc.x, ndc.y, 0.0, 1.0);
    vUV = inUV;
    vColor = inColor;
}
