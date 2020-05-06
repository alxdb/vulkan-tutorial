#version 450
#pragma shader_stage(vertex)
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec4 in_col;

layout(location = 0) out vec4 vx_col;

void main() {
	gl_Position = in_pos;
	vx_col = in_col;
}
