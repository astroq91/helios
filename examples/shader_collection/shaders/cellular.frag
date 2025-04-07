#version 450
#extension GL_EXT_nonuniform_qualifier : require

#define PI 3.14159265359

layout (location = 0) out vec4 outColor;

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
  vec2 mouse;
} p_stats;

vec2 random2f(vec2 p) {
  return fract(sin(vec2(dot(p, vec2(124.33, 38.43)), dot(p, vec2(128.89489, 819.32)))) * 2102.73829);
}


void main()
{
  vec3 color = vec3(0.0);

  vec2 st = v_in.frag_tex_coord;
  st *= 3.0;
  vec2 i_st = floor(st);
  vec2 f_st = fract(st);

  float min_dist = 1000.0;

  vec2 min_point;

  for (int y = -1; y <= 1; y++) {
    for (int x = -1; x <= 1; x++) {
      vec2 neighbour = vec2(float(x), float(y));
      vec2 point = random2f(i_st + neighbour);

      point = 0.5 + 0.5*sin(p_stats.time + 5.4253 * point);

      vec2 diff = point + neighbour - f_st;
      float dist = length(diff);

      if (dist < min_dist) {
        min_dist = dist;
        min_point = point;
      }
    }
  }

  color.rb = min_point;
  outColor = vec4(color, 1.0);
} 
