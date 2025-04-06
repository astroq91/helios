#version 450
#extension GL_EXT_nonuniform_qualifier : require

#define MAX_STEPS 100
#define MAX_DISTANCE 100.0 
#define MIN_DISTANCE 0.01 

layout (location = 0) out vec4 out_color;

layout (location = 0) in vec2 v_frag_tex_coord;
layout (location = 1) in vec3 v_frag_pos;
layout (location = 2) in vec3 v_normal;

layout(location = 3) in flat int v_diffuse_index;
layout(location = 4) in flat int v_specular_index;
layout(location = 5) in flat int v_emission_index;
layout(location = 6) in flat float v_shininess;

layout(location = 7) in vec4 v_tint_color;

layout(location = 8) in vec3 v_view_pos;

layout(set = 1, binding = 0) uniform sampler u_samp;
layout (set = 1, binding = 1) uniform texture2D u_textures[1000];

layout (push_constant) uniform PushConstants {
	float time;
  vec2 resolution;
  vec2 mouse;
} p_stats;

float sd_sphere(vec3 pos, float radius) {
  return length(pos) - radius;
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

  vec3 ro = vec3(0.0, 0.0, 3.0);
  vec3 rd = normalize(vec3(st, -1.0));
  //vec3 ro = v_view_pos;
  //vec3 rd = normalize(v_frag_pos - v_view_pos);
  vec3 light_pos = vec3(0.0, 0.0, 3.0);

  float t = raymarch(ro, rd);
  vec3 pos = ro + rd * t;

  if (t < MAX_DISTANCE) {
    vec3 normal = get_normal(pos);
    vec3 ld = normalize(light_pos - pos);
    color = vec3(max(dot(normal, ld), 0.0));
  }
  
  out_color = vec4(color, 1.0);
} 
