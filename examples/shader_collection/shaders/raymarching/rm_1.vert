#version 450

// Per vertex data
layout (location = 0) in vec3 i_position;
layout (location = 1) in vec3 i_normal;
layout (location = 2) in vec2 i_tex_coord;

// Per instance data
layout(location = 3) in mat4 i_model;

layout(location = 7) in int i_diffuse_index;
layout(location = 8) in int i_specular_index;
layout(location = 9) in int i_emission_index;
layout(location = 10) in float i_shininess;

layout(location = 11) in vec4 i_tint_color;


layout (location = 0) out vec2 o_frag_tex_coord;
layout (location = 1) out vec3 o_frag_pos;
layout (location = 2) out vec3 o_normal;

layout(location = 3) out int o_diffuse_index;
layout(location = 4) out int o_specular_index;
layout(location = 5) out int o_emission_index;
layout(location = 6) out float o_shininess;

layout(location = 7) out vec4 o_tint_color;

layout(location = 8) out vec3 o_view_pos;

layout(set = 0, binding = 0) uniform CameraUniform {
	mat4 perspective_view_proj;
	mat4 perspective_proj;
	mat4 perspective_view_no_translation;
	vec3 perspective_pos; 

	mat4 orthographic_proj;
} u_camera;

void main() {
  gl_Position = u_camera.perspective_view_proj * i_model * vec4(i_position, 1.0);

  o_frag_tex_coord = i_tex_coord;
  o_frag_pos = vec3(i_model * vec4(i_position, 1.0));
  o_normal = normalize(mat3(i_model) * i_normal);

  o_diffuse_index = i_diffuse_index;
  o_specular_index = i_specular_index;
  o_emission_index = i_emission_index;
  o_shininess = i_shininess;
  o_tint_color = i_tint_color;

  o_view_pos = u_camera.perspective_pos;
}
