#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) out vec4 outColor;

struct DirLight {
	vec3 Direction;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};

struct PointLight {
	vec3 Position;

	float Constant;
	float Linear;
	float Quadratic;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};

struct Material {
	int diffuseTextureUnit;
	int specularTextureUnit;
	int emissionTextureUnit;
	float shininess;
};

#define MAX_DIR_LIGHTS 32
#define MAX_POINT_LIGHTS 32
#define MAX_TEXTURES 32


layout (location = 0) in vec4 inFragColor;
layout (location = 1) in vec2 inFragTexCoord;
layout (location = 2) in vec3 inFragPos;
layout (location = 3) in vec3 inNormal;

layout(location = 4) in flat int inDiffuseIndex;
layout(location = 5) in flat int inSpecularIndex;
layout(location = 6) in flat int inEmissionIndex;
layout(location = 7) in flat float inShininess;

layout(location = 8) in vec3 viewPos;

layout(location = 9) in vec4 tintColor;


layout(set = 1, binding = 0) uniform sampler samp;
layout (set = 1, binding = 1) uniform texture2D textures[32];

layout(set = 2, binding = 0) uniform DirectionalLights {
	DirLight lights[MAX_DIR_LIGHTS];
} uDirectionalLights;

layout(set = 2, binding = 1) uniform PointLights {
	PointLight lights[MAX_POINT_LIGHTS];
} uPointLights;

layout (push_constant) uniform LightCount {
	int directional;
	int point;
} pLightCount;

void main()
{
  outColor = vec4(1.0, 0.0, 0.0, 1.0);
} 

