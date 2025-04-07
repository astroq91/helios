#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) out vec4 out_color;

layout(location = 0) in VertexInput {
  vec2 tex_coord;
  flat int texture_index;
  vec4 tint_color;
} v_in;

layout(set = 0, binding = 0) uniform sampler u_samp;
layout (set = 0, binding = 1) uniform texture2D u_textures[1000];

void main()
{
    vec4 tex_color = texture(sampler2D(u_textures[nonuniformEXT(v_in.texture_index)], u_samp), v_in.tex_coord);
    out_color = v_in.tint_color * vec4(tex_color.r, tex_color.r, tex_color.r, 1.0);
} 
