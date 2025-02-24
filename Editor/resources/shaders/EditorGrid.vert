#version 450

layout (set = 0, binding = 0) uniform Camera {
  mat4 view;
  mat4 proj;
  vec3 pos;
} uCamera;

layout (location = 0) out vec3 worldPos;
layout (location = 1) out float _gridSize;
layout (location = 2) out vec3 cameraPos;

const vec3 vertices[4] = vec3[4](
    vec3(-1.0, 0.0, -1.0),
    vec3(1.0, 0.0, -1.0),
    vec3(1.0, 0.0, 1.0),
    vec3(-1.0, 0.0, 1.0)
);

const int indices[6] = int[6](0, 2, 1, 2, 0, 3);

float gridSize = 100.0;

void main() {
  int index = indices[gl_VertexIndex];
  vec3 vPos = vertices[index] * gridSize;

  vPos.x += uCamera.pos.x;
  vPos.z += uCamera.pos.z;

  vec4 vPos4 = vec4(vPos, 1.0);

  gl_Position = uCamera.proj * uCamera.view * vPos4;

  worldPos = vPos;
  _gridSize = gridSize;
  cameraPos = uCamera.pos;
}
