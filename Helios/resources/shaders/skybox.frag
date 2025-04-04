#version 450

layout (location = 0) out vec4 out_color;
layout (location = 0) in vec3 v_tex_coords;

layout (set = 0, binding = 0) uniform samplerCube skybox;

void main() {
  out_color = texture(skybox, v_tex_coords);
}
