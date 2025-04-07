#version 450

layout (location = 0) in vec3 iv_position;

layout (location = 0) out vec3 v_tex_coords;

layout(set = 1, binding = 0) uniform CameraUniform {
	mat4 perspective_view_proj;
	mat4 perspective_proj;
	mat4 perspective_view_no_translation;
	vec3 perspective_pos; 

	mat4 orthographic_proj;
} u_camera;

void main() 
{
  v_tex_coords = iv_position;
	gl_Position = u_camera.perspective_proj * u_camera.perspective_view_no_translation * vec4(iv_position, 1.0);
}
