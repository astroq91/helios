#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoord;

layout (location = 2) in mat4 inModel;

layout (location = 6) in int inTextureIndex;
layout (location = 7) in vec4 inTintColor;

layout (location = 0) out vec2 outTexCoord;
layout (location = 1) out int outTextureIndex;
layout (location = 2) out vec4 outTintColor;


void main() 
{
	outTexCoord = inTexCoord;
	outTextureIndex = inTextureIndex;
	outTintColor = inTintColor;
	gl_Position = vec4(inPosition, 1.0);
}