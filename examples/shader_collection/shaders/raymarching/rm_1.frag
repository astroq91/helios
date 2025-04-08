#version 450
#extension GL_EXT_nonuniform_qualifier : require

#define MAX_STEPS 100
#define MAX_DISTANCE 100.0 
#define MIN_DISTANCE 0.01 

layout (location = 0) out vec4 out_color;

layout (location = 0) in vec2 v_frag_tex_coord;

layout(set = 1, binding = 0) uniform sampler u_samp;
layout (set = 1, binding = 1) uniform texture2D u_textures[1000];

layout (push_constant) uniform PushConstants {
	float time;
  vec2 resolution;
  vec2 mouse;
} p_stats;

float random(in vec3 v) {
  return fract(sin(dot(v, vec3(1.213, 2.88291, 18.12892))) * 127389.12838);
}

float noise(in vec3 v) {
  vec3 i_st = floor(v);
  vec3 f_st = fract(v);

  // Get a random value for each corner
  float a = random(i_st);
  float b = random(i_st + vec3(1.0, 0.0, 0.0));
  float c = random(i_st + vec3(0.0, 1.0, 0.0));
  float d = random(i_st + vec3(1.0, 1.0, 0.0));
  float e = random(i_st + vec3(0.0, 0.0, 1.0));
  float f = random(i_st + vec3(1.0, 0.0, 1.0));
  float g = random(i_st + vec3(0.0, 1.0, 1.0));
  float h = random(i_st + vec3(1.0, 1.0, 1.0));


  // 3d smoothing function
  vec3 u = f_st * f_st * (3.0 - 2.0 * f_st);

  // Mix the random values for each corner
  float x1 = mix(a, b, u.x);
  float x2 = mix(c, d, u.x);
  float y1 = mix(x1, x2, u.y);

  float x3 = mix(e, f, u.x);
  float x4 = mix(g, h, u.x);
  float y2 = mix(x3, x4, u.y);

  return mix(y1, y2, u.z);
}

float fbm(in vec3 v) {
  const int k_octaves = 5;
  const float k_gain = 0.5;
  const float k_lacunarity = 1.3;

  float value = 0.0;
  float amplitude = 1.5;
  float frequency = 1.4;
  for (int i = 0; i < k_octaves; i++) {
    value += amplitude * noise(v * frequency * smoothstep(0.0, 1.0, abs(sin(p_stats.time * 0.1))) * 0.9);
    frequency *= k_lacunarity;
    amplitude *= k_gain;
  }

  return value;
}

float fbm_2(in vec3 v) {
  vec3 p = vec3(fbm(v + vec3(0.288, 1.489, 1.2898)),
                fbm(v + vec3(2.389, 2.3, 6.88)),
                fbm(v + vec3(9.821, 8.4849, 4.21)));
  return fbm(v + p * 2.312);
}

float sd_sphere(vec3 pos, float radius) {
  return length(pos) - (radius + fbm_2(pos) * 0.1);
}

float scene(vec3 pos) {
  float distance = sd_sphere(pos, 1.0);
  return distance;
}

float raymarch(vec3 ro, vec3 rd) {
  float t = 0.0;
  for (int i = 0; i < MAX_STEPS; i++) {
    vec3 pos = ro + rd * t;
    float dist = scene(pos);
    t += dist;

    if (dist < MIN_DISTANCE || t > MAX_DISTANCE) {
      break;
    }
  }
  return t;
}

// https://iquilezles.org/articles/normalsSDF/
vec3 get_normal(vec3 pos) {
  vec2 e = vec2(.01, 0);

  vec3 n = scene(pos) - vec3(
    scene(pos-e.xyy),
    scene(pos-e.yxy),
    scene(pos-e.yyx));

  return normalize(n);
}

void main()
{
  vec3 color = vec3(0.0);
  vec2 st = v_frag_tex_coord - 0.5; // Move 0,0 to center
  st.x *= 7.0 / 7.0; // Aspect ratio

  vec2 mouse = p_stats.mouse / p_stats.resolution;
  mouse.y *= -1;

  vec3 ro = vec3(0.0, 0.0, 3.0);
  vec3 rd = normalize(vec3(st, -1.0));
  //vec3 ro = v_view_pos;
  //vec3 rd = normalize(v_frag_pos - v_view_pos);
  vec3 light_pos = vec3(mouse * 4, 3.0);

  float t = raymarch(ro, rd);
  vec3 pos = ro + rd * t;

  if (t < MAX_DISTANCE) {
    vec3 normal = get_normal(pos);
    vec3 ld = normalize(light_pos - pos);
    color = vec3(max(dot(normal, ld), 0.0));
  }
  
  out_color = vec4(color, 1.0);
} 
