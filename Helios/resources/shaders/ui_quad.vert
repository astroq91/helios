#version 450

layout (location = 0) in vec2 inVertexPosition;
layout (location = 1) in vec2 inTexCoord;

layout (location = 2) in mat4 inModel;

layout (location = 6) in vec4 inTintColor;
layout (location = 7) in int inTextureIndex;

layout (location = 0) out vec2 outTexCoord;
layout (location = 1) out flat int outTextureIndex;
layout (location = 2) out vec4 outTintColor;

layout(set = 1, binding = 0) uniform CameraUniform {
	mat4 perspective_view_proj;
	vec3 perspective_pos; 

	mat4 orthographic_proj;
} uCamera;

void main() 
{
	outTexCoord = inTexCoord;
	outTextureIndex = inTextureIndex;
	outTintColor = inTintColor;
	gl_Position = uCamera.orthographic_proj * inModel * vec4(inVertexPosition, 0.0, 1.0);
}