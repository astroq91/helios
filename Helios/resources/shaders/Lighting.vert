#version 450

// Per-vertex data
layout(location = 0) in vec3 iv_position;
layout(location = 1) in vec3 iv_normal;
layout(location = 2) in vec2 iv_tex_coord;

// Per-instance data
layout(location = 3) in mat4 ii_model;

layout(location = 7) in int ii_diffuse_index;
layout(location = 8) in int ii_specular_index;
layout(location = 9) in int ii_emission_index;
layout(location = 10) in float ii_shininess;

layout(location = 11) in vec4 ii_tint_color;

layout(set = 0, binding = 0) uniform CameraUniform {
    mat4 perspective_view_proj;
    mat4 perspective_proj;
    mat4 perspective_view_no_translation;
    vec3 perspective_pos;

    mat4 orthographic_proj;
} u_camera;

layout(location = 0) out VertexInput {
    vec2 frag_tex_coord;
    vec3 frag_pos;
    vec3 normal;
    flat int diffuse_index;
    flat int specular_index;
    flat int emission_index;
    flat float shininess;
    vec4 tint_color;
    vec3 view_pos;
} v_out;

void main() {
    gl_Position = u_camera.perspective_view_proj * ii_model * vec4(iv_position, 1.0);
    v_out.frag_tex_coord = iv_tex_coord;
    v_out.frag_pos = vec3(ii_model * vec4(iv_position, 1.0));

    // Get rotation/scale part of the model matrix
    mat3 normal_matrix = mat3(ii_model);
    v_out.normal = normalize(normal_matrix * iv_normal);

    v_out.diffuse_index = ii_diffuse_index;
    v_out.specular_index = ii_specular_index;
    v_out.emission_index = ii_emission_index;
    v_out.shininess = ii_shininess;
    v_out.tint_color = ii_tint_color;
    v_out.view_pos = u_camera.perspective_pos;
}

