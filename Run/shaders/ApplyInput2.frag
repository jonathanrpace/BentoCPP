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
	ivec2 T = ivec2(gl_FragCoord.xy);

	srand(in_uv.x * in_uv.y);
	
	vec2 mousePos = GetMousePos();

	vec2 offset = mousePos - in_uv;
	float d = length(offset);
	float mouseRatio = 1.0f - min(1.0f, d / u_mouseRadius);
	
	float addedMolten = mouseRatio * u_mouseMoltenVolumeStrength;
	float addedWater = mouseRatio * u_mouseWaterVolumeStrength;
	
	vec4 heightDataC = texelFetch( s_heightData, T, 0 );
	heightDataC.y += addedMolten;
	heightDataC.w += addedWater;
	out_heightData = heightDataC;
	
	vec4 fluidVelocityC = texelFetch( s_fluidVelocity, T, 0 ); 
	vec2 addedMoltenVelocity = offset * mouseRatio * rand();// * u_mouseMoltenVolumeStrength;// * rand();
	vec2 addedWaterVelocity = offset * mouseRatio * rand();// * u_mouseWaterVolumeStrength;// * rand();
	
	fluidVelocityC.xy -= addedMoltenVelocity * 1.0 * u_mouseMoltenHeatStrength;
	fluidVelocityC.zw -= addedWaterVelocity * 1.0 * u_mouseMoltenHeatStrength;

	
	// Subtract height gradient
	{
		vec4 hN = texelFetchOffset(s_heightData, T, 0, ivec2( 0, -1));
		vec4 hS = texelFetchOffset(s_heightData, T, 0, ivec2( 0,  1));
		vec4 hE = texelFetchOffset(s_heightData, T, 0, ivec2( 1,  0));
		vec4 hW = texelFetchOffset(s_heightData, T, 0, ivec2(-1,  0));

		float mhN = hN.x + hN.y;
		float mhS = hS.x + hS.y;
		float mhE = hE.x + hE.y;
		float mhW = hW.x + hW.y;

		vec2 heightGradientMolten = vec2(mhE - mhW, mhS - mhN);

		fluidVelocityC.xy -= heightGradientMolten * 1.0;// * 100000000.0;//u_gradientScale;
	}
	
	out_fluidVelocity = fluidVelocityC;
}