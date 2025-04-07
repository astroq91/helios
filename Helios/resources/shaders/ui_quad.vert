#version 450

layout (location = 0) in vec2 iv_position;
layout (location = 1) in vec2 iv_tex_coord;

layout (location = 2) in mat4 ii_model;

layout (location = 6) in vec4 ii_tint_color;
layout (location = 7) in int ii_texture_index;

layout(location = 0) out VertexOutput {
  vec2 tex_coord;
  flat int texture_index;
  vec4 tint_color;
} v_out;

layout(set = 1, binding = 0) uniform CameraUniform {
  mat4 perspective_view_proj;
	mat4 perspective_proj;
	mat4 perspective_view_no_translation;
	vec3 perspective_pos; 

	mat4 orthographic_proj;
} u_camera;

void main() 
{
	v_out.tex_coord = iv_tex_coord;
	v_out.texture_index = ii_texture_index;
	v_out.tint_color = ii_tint_color;
	gl_Position = u_camera.orthographic_proj * ii_model * vec4(iv_position, 0.0, 1.0);
}
