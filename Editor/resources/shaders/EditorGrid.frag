#version 450

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in float inGridSize;
layout (location = 2) in vec3 inCameraPos;

float gridMinPixelsBetweenCells = 2.0;
float gridCellSize = 0.025;
vec4 gridColorThin = vec4(0.5, 0.5, 0.5, 1.0);
vec4 gridColorThick = vec4(0.0, 0.0, 0.0, 1.0);

float log10(float x) {
  return log(x) / log(10.0);
}

float satf(float x) {
  return clamp(x, 0.0, 1.0);
}

vec2 satv(vec2 v) {
    return clamp(v, 0.0, 1.0);
}

float max2(vec2 v) {
  return max(v.x, v.y);
}

void main() {
  vec2 dvx = vec2(dFdx(inWorldPos.x), dFdy(inWorldPos.x));
  vec2 dvy = vec2(dFdx(inWorldPos.z), dFdy(inWorldPos.z));

  float lx = length(dvx);
  float ly = length(dvy);

  vec2 dudv = vec2(lx, ly);
  dudv *= 4;

  float l = length(dudv);

  float lod = max(0.0, log10(l * gridMinPixelsBetweenCells / gridCellSize) + 1.0);

  float gridCellSizeLod0 = gridCellSize * pow(10.0, floor(lod));
  float gridCellSizeLod1 = gridCellSizeLod0 * 10.0;
  float gridCellSizeLod2 = gridCellSizeLod1 * 10.0;

  vec2 modDivDudv = mod(inWorldPos.xz, gridCellSizeLod0) / dudv;
  float lod0a = max2(vec2(1.0) - abs(satv(modDivDudv) * 2.0 - vec2(1.0)));

  modDivDudv = mod(inWorldPos.xz, gridCellSizeLod1) / dudv;
  float lod1a = max2(vec2(1.0) - abs(satv(modDivDudv) * 2.0 - vec2(1.0)));

  modDivDudv = mod(inWorldPos.xz, gridCellSizeLod2) / dudv;
  float lod2a = max2(vec2(1.0) - abs(satv(modDivDudv) * 2.0 - vec2(1.0)));

  float lodFade = fract(lod);

  vec4 color;

  if (lod2a > 0.0) {
    color = gridColorThick;
    color.a *= lod2a;
  } else {
    if (lod1a > 0.0) {
      color = mix(gridColorThick, gridColorThin, lodFade);
      color.a *= lod1a;
    } else {
      color = gridColorThin;
      color.a *= (lod0a * (1.0 - lodFade));
    }
  }

  float opacityFalloff = (1.0 - satf(length(inWorldPos.xz - inCameraPos.xz) / inGridSize));
  color.a *= opacityFalloff;

  outColor = color;
}
