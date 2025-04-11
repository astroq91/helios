#version 450

// Per vertex data
layout (location = 0) in vec3 iv_position;
layout (location = 1) in vec3 iv_normal;
layout (location = 2) in vec2 iv_tex_coord;

// Per instance data
layout(location = 3) in mat4 ii_model;

layout(location = 7) in int ii_diffuse_index;
layout(location = 8) in int ii_specular_index;
layout(location = 9) in int ii_emission_index;
layout(location = 10) in float ii_shininess;

layout(location = 11) in vec4 ii_tint_color;

layout (location = 0) out vec2 o_frag_tex_coord;

layout(set = 0, binding = 0) uniform CameraUniform {
	mat4 perspective_view_proj;
	mat4 perspective_proj;
	mat4 perspective_view_no_translation;
	vec3 perspective_pos; 

	mat4 orthographic_proj;
} u_camera;

void main() {
  gl_Position = u_camera.perspective_view_proj * ii_model * vec4(iv_position, 1.0);

  o_frag_tex_coord = iv_tex_coord;
}
