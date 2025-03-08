#version 450

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec2 inTexCoord;
layout (location = 1) flat in int inTextureIndex;
layout (location = 2) in vec4 inTintColor;

layout(set = 0, binding = 0) uniform sampler samp;
layout (set = 0, binding = 1) uniform texture2D textures[1000];

vec3 linearizeSRGB(vec3 srgbColor) {
    return mix(srgbColor / 12.92, pow((srgbColor + vec3(0.055)) / 1.055, vec3(2.4)), step(vec3(0.04045), srgbColor));
}

vec4 linearizeSRGB(vec4 srgbColor) {
    return vec4(linearizeSRGB(srgbColor.rgb), srgbColor.a);
}


void main()
{
	outColor = linearizeSRGB(inTintColor) * texture(sampler2D(textures[inTextureIndex], samp), inTexCoord);
} 