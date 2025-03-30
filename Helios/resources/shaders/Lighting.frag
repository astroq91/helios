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
layout(location = 8) in vec4 tintColor;

layout(location = 9) in vec3 viewPos;

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


vec3 linearizeSRGB(vec3 srgbColor) {
    return mix(srgbColor / 12.92, pow((srgbColor + vec3(0.055)) / 1.055, vec3(2.4)), step(vec3(0.04045), srgbColor));
}

vec4 linearizeSRGB(vec4 srgbColor) {
    return vec4(linearizeSRGB(srgbColor.rgb), srgbColor.a);
}

vec3 delinearizeSRGB(vec3 linearColor) {
    return mix(12.92 * linearColor, 1.055 * pow(linearColor, vec3(1.0 / 2.4)) - vec3(0.055), step(vec3(0.0031308), linearColor));
}

vec4 delinearizeSRGB(vec4 linearColor) {
    return vec4(delinearizeSRGB(linearColor.rgb), linearColor.a);
}

vec3 CalcDirLight(DirLight light, vec3 diffuseTexture, vec3 specularTexture, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 diffuseTexture, vec3 specularTexture, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 viewDir = normalize(viewPos - inFragPos);

	vec3 diffuseTexture = vec3(texture(sampler2D(textures[nonuniformEXT(inDiffuseIndex)], samp), inFragTexCoord));
	vec3 specularTexture = vec3(texture(sampler2D(textures[nonuniformEXT(inSpecularIndex)], samp), inFragTexCoord));
	//vec4 emissionTexture = texture(u_Textures[cubeInstances[InstanceID].material.emissionTextureUnit], TexCoord);

	vec3 result = vec3(0.0);
	// Directional lighting
	for (int i = 0; i < pLightCount.directional; i++) {
		result += CalcDirLight(uDirectionalLights.lights[i], diffuseTexture, specularTexture, inNormal, viewDir);
	}

	// Point lights
	for (int i = 0; i < pLightCount.point; i++) {
		result += CalcPointLight(uPointLights.lights[i], diffuseTexture, specularTexture, inNormal, inFragPos, viewDir);
	}
	
	// TODO: Spot lights...

	// Flashlight
	//float theta = dot(lightDir, normalize(-u_Light.Direction));
	//float epsilon = u_Light.CutOff - u_Light.OuterCutOff;
	//float intensity = clamp((theta - u_Light.OuterCutOff) / epsilon, 0.0, 1.0);

	//diffuse *= intensity;
	//specular *= intensity;

	outColor = delinearizeSRGB(vec4(result, 1.0)) * tintColor;
  //outColor = vec4(normalize(inNormal) * 0.5 + 0.5, 1.0);
} 

vec3 CalcDirLight(DirLight light, vec3 diffuseTexture, vec3 specularTexture, vec3 normal, vec3 viewDir) 
{
	vec3 lightDir = normalize(-light.Direction);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), inShininess);

	vec3 ambient = light.Ambient * diffuseTexture;
	vec3 diffuse = light.Diffuse * diff * diffuseTexture;
	vec3 specular =  light.Specular * spec * specularTexture;

	return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 diffuseTexture, vec3 specularTexture, vec3 normal, vec3 fragPos, vec3 viewDir) 
{
	vec3 lightDir = normalize(light.Position - fragPos);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), inShininess);
	
	// Attenuation
	float distance = length(light.Position - fragPos);
	float attenuation = 1.0 / (light.Constant + light.Linear * distance + light.Quadratic * (distance * distance));

	vec3 ambient = light.Ambient * diffuseTexture;
	vec3 diffuse = light.Diffuse * diff * diffuseTexture;
	vec3 specular = light.Specular * spec * specularTexture;

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}
