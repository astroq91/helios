#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec4 v_frag_color;
layout (location = 1) in vec2 v_frag_tex_coord;
layout (location = 2) in vec3 v_frag_pos;
layout (location = 3) in vec3 v_normal;

layout(location = 4) in flat int v_diffuse_index;
layout(location = 5) in flat int v_specular_index;
layout(location = 6) in flat int v_emission_index;
layout(location = 7) in flat float v_shininess;

layout(location = 8) in vec4 v_tint_color;

layout(set = 1, binding = 0) uniform sampler u_samp;
layout (set = 1, binding = 1) uniform texture2D u_textures[32];

layout (push_constant) uniform PushConstants {
	float time;
  vec2 resolution;
} p_stats;

void main()
{
  outColor = vec4(v_frag_tex_coord, 0.0, 1.0);
} 

