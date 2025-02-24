#version 450

layout (location = 0) out vec4 outColor;

layout (location = 0) in flat int inEntityId;

#define UINT32_MAX 0xffffffff

void main()
{
	if (inEntityId == UINT32_MAX) {
		outColor = vec4(1.0); // White for no entity
	}
	else {
		int r = (inEntityId & 0x000000FF) >>  0;
		int g = (inEntityId & 0x0000FF00) >>  8;
		int b = (inEntityId & 0x00FF0000) >> 16;
		outColor = vec4(float(r) / 255.0, float(g) / 255.0, float(b) / 255.0, 1.0);
	}
} 