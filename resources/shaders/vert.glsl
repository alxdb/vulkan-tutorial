#version 450

layout(location = 0) in vec2 pos;

layout(push_constant) uniform PushConstantData {
    vec2 transform;
} pc;

void main() {
    gl_Position = vec4(pos + pc.transform, 0.0, 1.0);
}
