#version 450
#extension GL_EXT_nonuniform_qualifier : require

#define PI 3.14159265359

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
  vec2 mouse;
} p_stats;

float random(in vec2 st) {
  return fract(sin(dot(st, vec2(3289.8,28891.3487))) * 538974.3248);
}

float noise(in vec2 st) {
  vec2 i_st = floor(st);
  vec2 f_st = fract(st);

  // Get a random value for each corner
  float a = random(i_st);
  float b = random(i_st + vec2(1.0, 0.0));
  float c = random(i_st + vec2(0.0, 1.0));
  float d = random(i_st + vec2(1.0, 1.0));

  // 2d smoothing function
  vec2 u = f_st * f_st * (3.0 - 2.0 * f_st);

  // Mix the random values for each corner
  float bottom = mix(a, b, u.x);
  float top = mix(c, d, u.x);
  return mix(bottom, top, u.y);;
}

float fbm(in vec2 st) {
  const int k_octaves = 6;
  const float k_gain = 0.4;
  const float k_lacunarity = 3.0;

  float value = 0.0;
  float amplitude = 0.8;
  float frequency = 1.2 * sin(p_stats.time * 0.128);
  for (int i = 0; i < k_octaves; i++) {
    value += amplitude * noise(st * frequency);
    frequency *= k_lacunarity;
    amplitude *= k_gain;
  }

  return value;
}

float fbm_warped(in vec2 p, out vec2 q, out vec2 r) {
  q.xy = vec2(fbm(p + vec2(0.3, 1.3)), 
                fbm(p + vec2(3.8, 9.4)));

  r.xy = vec2(fbm(p + 3.0*q + vec2(2.3, 8.3)), 
                fbm(p + 3.0*q + vec2(5.3, 1.4)));
  return fbm(p + 2.4 * r);
}

void main()
{
  vec3 color = vec3(0.1, 0.05, 0.3);

  vec2 st = v_frag_tex_coord;
  st.x *= 12.5/8.0; // Aspect ratio (based on screen object size)
  st *= 3.0;

  vec2 q;
  vec2 r;
  float f = fbm_warped(st, q, r);
  color = mix(color, vec3(0.8, 0.45, 0.22), f);
  color = mix(color, vec3(0.11, 0.30, 0.90), 0.8*smoothstep(1.3, 1.4, abs(q.y)+abs(q.x)));

  outColor = vec4(color, 1.0);
} 
