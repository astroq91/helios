#version 450

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec4 fragColor;
layout (location = 1) in vec2 fragTexCoord;

layout(set = 1, binding = 0) uniform sampler samp;
layout (set = 1, binding = 1) uniform texture2D textures[32];

layout (push_constant) uniform PushConstant {
	int textureIndex;
} pushConstant;


vec3 linearizeSRGB(vec3 srgbColor) {
    return mix(srgbColor / 12.92, pow((srgbColor + vec3(0.055)) / 1.055, vec3(2.4)), step(vec3(0.04045), srgbColor));
}

vec4 linearizeSRGB(vec4 srgbColor) {
    return vec4(linearizeSRGB(srgbColor.rgb), srgbColor.a);
}


void main()
{
	outColor = linearizeSRGB(fragColor) * texture(sampler2D(textures[pushConstant.textureIndex], samp), fragTexCoord);
} 