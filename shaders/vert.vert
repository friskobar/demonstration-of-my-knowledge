#version 450

layout(location = 0)in vec2 vert_pos;
layout(location = 1)in vec3 vert_color;

layout(location = 0) out vec3 frag_color;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(vert_pos, 0.0, 1.0);
    frag_color = vert_color;
}
