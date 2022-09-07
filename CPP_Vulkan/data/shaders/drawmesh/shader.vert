#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 tex_coords;

layout(location = 0) out vec3 frag_color;

//push constants block
layout( push_constant ) uniform constants
{
	vec4 data;
	mat4 render_matrix;
} push_constants;


void main() 
{
	gl_Position = push_constants.render_matrix * vec4(position, 1.0);
	frag_color = normal;
	//frag_color = vec3(1);
}