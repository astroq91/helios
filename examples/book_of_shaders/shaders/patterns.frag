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


float plot(vec2 st) {
  return smoothstep(0.02, 0.0, abs(st.y - st.x));
}

float circle(in vec2 _st, in vec2 _center, in float _radius){
    vec2 dist = _st-vec2(_center);
	return 1.-smoothstep(_radius-(_radius*0.01),
                         _radius+(_radius*0.01),
                         dot(dist,dist)*4.0);
}

void main()
{
  vec3 color = vec3(0.0);
  vec2 mouse = p_stats.mouse / p_stats.resolution;
  mouse.y = 1.0 - mouse.y;

  vec2 uv = v_frag_tex_coord;
  uv *= 200;
  uv.x += step(1.0, mod(uv.y, 2.0)) * fract(p_stats.time);
  uv = fract(uv);

  color = vec3(circle(uv, vec2(0.5), 0.5)) * vec3(v_frag_tex_coord, 1.0);

  outColor = vec4(color, 1.0);
} 
