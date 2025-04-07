#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) out vec4 out_color;

layout(location = 0) in VertexInput {
    vec2 frag_tex_coord;
    vec3 frag_pos;
    vec3 normal;
    flat int diffuse_index;
    flat int specular_index;
    flat int emission_index;
    flat float shininess;
    vec4 tint_color;
    vec3 view_pos;
} v_in;

layout(set = 1, binding = 0) uniform sampler u_samp;
layout (set = 1, binding = 1) uniform texture2D u_textures[32];

layout (push_constant) uniform PushConstants {
	float time;
  vec2 resolution;
} p_stats;

void main()
{
  out_color = vec4(v_in.frag_tex_coord, 0.0, 1.0);
} 

