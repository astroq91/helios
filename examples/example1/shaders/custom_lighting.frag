#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) out vec4 outColor;

struct Material {
	int diffuseTextureUnit;
	int specularTextureUnit;
	int emissionTextureUnit;
	float shininess;
};

layout (location = 0) in vec4 inFragColor;
layout (location = 1) in vec2 inFragTexCoord;
layout (location = 2) in vec3 inFragPos;
layout (location = 3) in vec3 inNormal;

layout(location = 4) in flat int inDiffuseIndex;
layout(location = 5) in flat int inSpecularIndex;
layout(location = 6) in flat int inEmissionIndex;
layout(location = 7) in flat float inShininess;

layout(location = 8) in vec4 tintColor;

layout(set = 1, binding = 0) uniform sampler samp;
layout (set = 1, binding = 1) uniform texture2D textures[32];

layout (push_constant) uniform LightCount {
	float alpha;
} pLightCount;

void main()
{
  outColor = vec4(inFragTexCoord, 0.0, 1.0);
} 

