#version 430 core

// From VS
in Varying
{
	vec2 in_uv;
};

void srand(float seed);
float rand();

layout( location = 0 ) out vec4 out_heightData;
layout( location = 1 ) out vec4 out_fluidVelocity;

// Buffers
layout( std430, binding = 0 ) buffer MousePositionBuffer
{
	int mouseBufferZ;
	int mouseBufferU;
	int mouseBufferV;
};

uniform vec2 u_mousePos;

uniform float u_mouseRadius;
uniform float u_mouseMoltenVolumeStrength;
uniform float u_mouseWaterVolumeStrength;
uniform float u_mouseMoltenHeatStrength;
uniform float u_mouseDirtVolumeStrength;

uniform sampler2D s_fluidVelocity;
uniform sampler2D s_heightData;


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
	float mouseRatio = 1.0f - min(1.0f, d / u_mouseRadius);
	
	float addedMolten = mouseRatio * u_mouseMoltenVolumeStrength;
	float addedWater = mouseRatio * u_mouseWaterVolumeStrength;
	
	vec4 heightDataC = texelFetch( s_heightData, ivec2(gl_FragCoord.xy), 0 );
	heightDataC.y += addedMolten;
	heightDataC.w += addedWater;
	out_heightData = heightDataC;
	
	vec4 fluidVelocityC = texelFetch( s_fluidVelocity, ivec2(gl_FragCoord.xy), 0 ); 
	vec2 addedMoltenVelocity = offset * mouseRatio * rand();// * u_mouseMoltenVolumeStrength;// * rand();
	vec2 addedWaterVelocity = offset * mouseRatio * rand();// * u_mouseWaterVolumeStrength;// * rand();
	
	fluidVelocityC.xy -= addedMoltenVelocity * 10.0 * u_mouseMoltenHeatStrength;
	fluidVelocityC.zw -= addedWaterVelocity * 10.0 * u_mouseMoltenHeatStrength;
	
	out_fluidVelocity = fluidVelocityC;
}