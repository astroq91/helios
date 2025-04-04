#version 450

layout (location = 0) in vec3 inVertexPosition;

layout (location = 0) out vec3 v_tex_coords;

layout(set = 1, binding = 0) uniform CameraUniform {
	mat4 perspective_view_proj;
	vec3 perspective_pos; 

	mat4 orthographic_proj;
} uCamera;

void main() 
{
  v_tex_coords = inVertexPosition;
	gl_Position = uCamera.perspective_view_proj * vec4(inVertexPosition, 1.0);
}
