#version 450
#extension GL_ARB_separate_shader_objects : enable
#pragma shader_stage(fragment)

layout(location = 0) in vec4 vx_color;

layout(location = 0) out vec4 ft_color;

void main() {
	ft_color = vx_color;
}
