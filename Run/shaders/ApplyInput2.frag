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
layout( location = 2 ) out vec4 out_miscData;

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
uniform sampler2D s_miscData;


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
	vec4 hC = heightDataC;

	heightDataC.y += addedMolten;
	heightDataC.x += addedWater;
	out_heightData = heightDataC;
	
	

	// Add heat
	vec4 miscDataC = texelFetch( s_miscData, T, 0 );
	float heat = min( miscDataC.x, 1.0 );
	heat += mouseRatio * u_mouseMoltenHeatStrength;
	miscDataC.x = heat;
	out_miscData = miscDataC;

	vec4 fluidVelocityC = texelFetch( s_fluidVelocity, T, 0 ); 
	vec2 addedMoltenVelocity = offset * mouseRatio;// * mix( 0.5, 1.0, rand());
	vec2 addedWaterVelocity = offset * mouseRatio;// * mix( 0.5, 1.0, rand());
	//fluidVelocityC.xy -= addedMoltenVelocity * 1.0 * u_mouseMoltenHeatStrength;
	//fluidVelocityC.zw -= addedWaterVelocity * 1.0 * u_mouseMoltenHeatStrength;

	// Subtract height gradient
	{
		vec4 hN = texelFetchOffset(s_heightData, T, 0, ivec2( 0, -1));
		vec4 hS = texelFetchOffset(s_heightData, T, 0, ivec2( 0,  1));
		vec4 hE = texelFetchOffset(s_heightData, T, 0, ivec2( 1,  0));
		vec4 hW = texelFetchOffset(s_heightData, T, 0, ivec2(-1,  0));

		float mhC = hC.x + hC.y;
		float mhN = hN.x + hN.y;
		float mhS = hS.x + hS.y;
		float mhE = hE.x + hE.y;
		float mhW = hW.x + hW.y;

		vec2 heightGradientMolten = vec2(mhE - mhW, mhS - mhN);

		fluidVelocityC.xy -= heightGradientMolten * 0.2;
	}

	// No velocity where there's no molten
	fluidVelocityC.xy *= smoothstep( 0.0, 0.001, heightDataC.y );

	// Slow velocity based on viscosity
	float moltenViscosity = pow( heat, 0.5 );

	fluidVelocityC.xy = mix( fluidVelocityC.xy, vec2(0.0,0.0), 1.0-moltenViscosity );

	
	out_fluidVelocity = fluidVelocityC;
}