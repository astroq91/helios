#version 450

// Per vertex data
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoord;

// Per instance data
layout(location = 3) in mat4 inModel;

layout(location = 7) in int inDiffuseIndex;
layout(location = 8) in int inSpecularIndex;
layout(location = 9) in int inEmissionIndex;
layout(location = 10) in float inShininess;

layout(location = 11) in vec4 inTintColor;

layout (set = 0, binding = 0) uniform UniformBuffer {
    	mat4 view;
    	mat4 proj;
	vec3 pos;
} ubo;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragTexCoord;
layout (location = 2) out vec3 fragPos;
layout (location = 3) out vec3 normal;

layout(location = 4) out flat int diffuseIndex;
layout(location = 5) out flat int specularIndex;
layout(location = 6) out flat int emissionIndex;
layout(location = 7) out flat float shininess;

layout(location = 8) out vec3 viewPos;

layout(location = 9) out vec4 tintColor;

void main() 
{
	gl_Position = ubo.proj * ubo.view * inModel * vec4(inPosition, 1.0);
	fragColor = vec4(.4f);
	fragTexCoord = inTexCoord;
	fragPos = vec3(inModel * vec4(inPosition, 1.0));

  // Get the rot/scale part of the model matrix
  mat3 normalMatrix = mat3(inModel);
	normal = normalize(normalMatrix * inNormal);

	diffuseIndex = inDiffuseIndex;
	specularIndex = inSpecularIndex;
	emissionIndex = inEmissionIndex;
	shininess = inShininess;

  tintColor = inTintColor;

	viewPos = ubo.pos;
}
