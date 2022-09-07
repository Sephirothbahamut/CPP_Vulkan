#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 frag_color;

//push constants block
layout( push_constant ) uniform constants
{
	vec4 data;
	mat4 render_matrix;
} push_constants;

void main() {
    outColor = vec4(frag_color, 1);
}