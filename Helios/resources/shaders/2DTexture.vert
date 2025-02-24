#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoord;

layout (set = 0, binding = 0) uniform UniformBuffer {
	mat4 model;
    	mat4 view;
    	mat4 proj;
	vec3 color;
} ubo;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragTexCoord;

void main() 
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
	fragColor = vec4(ubo.color, 1.0f);
	fragTexCoord = inTexCoord;
}