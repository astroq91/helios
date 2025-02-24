#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord; // Needed for compatibility reasons

layout(location = 3) in mat4 inModel;

layout(location = 7) in uint inEntityId;

layout (set = 0, binding = 0) uniform UniformBuffer {
    	mat4 view;
    	mat4 proj;
      vec2 pos;
} ubo;

layout (location = 0) out flat uint entityId;

void main() 
{
	gl_Position = ubo.proj * ubo.view * inModel * vec4(inPosition, 1.0);

	entityId = inEntityId;
}
