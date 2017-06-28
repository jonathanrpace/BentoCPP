#version 430 core

// From VS
in Varying
{
	vec2 in_uv;
};

void srand(float seed);
float rand();

layout( location = 0 ) out vec4 out_fragColor0;
layout( location = 1 ) out vec4 out_fragColor1;

// Buffers
layout( std430, binding = 0 ) buffer MousePositionBuffer
{
	int mouseBufferZ;
	int mouseBufferU;
	int mouseBufferV;
};

uniform vec2 u_mousePos;
uniform float u_radius;
uniform float u_strengthA;
uniform float u_strengthB;



////////////////////////////////////////////////////////////////
//
vec2 GetMousePos()
{
	vec2 mousePos = vec2( mouseBufferU, mouseBufferV ) / 255;
	return mousePos;
}


void main()
{
	srand(in_uv.x * in_uv.y);
	
	vec2 mousePos = GetMousePos();
	vec2 offset = mousePos - in_uv;
	float d = length(offset);
	float mouseRatio = 1.0f - min(1.0f, d / u_radius);
	
	out_fragColor0 = vec4(mouseRatio * u_strengthA * 5.0);
	
	out_fragColor1 = -vec4(offset * mouseRatio * u_strengthB * rand() * 2.0, 0.0, 0.0);
}