#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) out vec4 out_color;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define MAX_DIR_LIGHTS 32
#define MAX_POINT_LIGHTS 32
#define MAX_TEXTURES 32

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
layout(set = 1, binding = 1) uniform texture2D u_textures[MAX_TEXTURES];

layout(set = 2, binding = 0) uniform DirectionalLights {
    DirLight lights[MAX_DIR_LIGHTS];
} u_directional_lights;

layout(set = 2, binding = 1) uniform PointLights {
    PointLight lights[MAX_POINT_LIGHTS];
} u_point_lights;

layout(push_constant) uniform LightCount {
    int directional;
    int point;
} u_light_count;

vec3 linearize_srgb(vec3 srgb_color) {
    return mix(srgb_color / 12.92, pow((srgb_color + vec3(0.055)) / 1.055, vec3(2.4)), step(vec3(0.04045), srgb_color));
}

vec4 linearize_srgb(vec4 srgb_color) {
    return vec4(linearize_srgb(srgb_color.rgb), srgb_color.a);
}

vec3 delinearize_srgb(vec3 linear_color) {
    return mix(12.92 * linear_color, 1.055 * pow(linear_color, vec3(1.0 / 2.4)) - vec3(0.055), step(vec3(0.0031308), linear_color));
}

vec4 delinearize_srgb(vec4 linear_color) {
    return vec4(delinearize_srgb(linear_color.rgb), linear_color.a);
}

vec3 calc_dir_light(DirLight light, vec3 diffuse_texture, vec3 specular_texture, vec3 normal, vec3 view_dir) {
    vec3 light_dir = normalize(-light.direction);

    float diff = max(dot(normal, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), v_in.shininess);

    vec3 ambient = light.ambient * diffuse_texture;
    vec3 diffuse = light.diffuse * diff * diffuse_texture;
    vec3 specular = light.specular * spec * specular_texture;

    return ambient + diffuse + specular;
}

vec3 calc_point_light(PointLight light, vec3 diffuse_texture, vec3 specular_texture, vec3 normal, vec3 frag_pos, vec3 view_dir) {
    vec3 light_dir = normalize(light.position - frag_pos);

    float diff = max(dot(normal, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), v_in.shininess);

    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * diffuse_texture * attenuation;
    vec3 diffuse = light.diffuse * diff * diffuse_texture * attenuation;
    vec3 specular = light.specular * spec * specular_texture * attenuation;

    return ambient + diffuse + specular;
}

void main() {
    vec3 view_dir = normalize(v_in.view_pos - v_in.frag_pos);

    vec3 diffuse_texture = vec3(texture(sampler2D(u_textures[nonuniformEXT(v_in.diffuse_index)], u_samp), v_in.frag_tex_coord));
    vec3 specular_texture = vec3(texture(sampler2D(u_textures[nonuniformEXT(v_in.specular_index)], u_samp), v_in.frag_tex_coord));

    vec3 result = vec3(0.0);

    for (int i = 0; i < u_light_count.directional; i++) {
        result += calc_dir_light(u_directional_lights.lights[i], diffuse_texture, specular_texture, v_in.normal, view_dir);
    }

    for (int i = 0; i < u_light_count.point; i++) {
        result += calc_point_light(u_point_lights.lights[i], diffuse_texture, specular_texture, v_in.normal, v_in.frag_pos, view_dir);
    }

    out_color = delinearize_srgb(vec4(result, 1.0)) * v_in.tint_color;
  }

