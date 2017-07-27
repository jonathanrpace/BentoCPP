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

uniform float u_moltenSlopeStrength;
uniform float u_waterGradientStrength = 0.3;

uniform float u_moltenVelocityDamping;

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
	float addedSolid = mouseRatio * u_mouseDirtVolumeStrength;
	
	vec4 hC = texelFetch( s_heightData, T, 0 );
	hC.x += addedSolid;
	hC.y += addedMolten;
	hC.w += addedWater;
	out_heightData = hC;
	
	vec4 fC = texelFetch( s_fluidVelocity, T, 0 ); 
	out_fluidVelocity = fC;
	
	// Add heat
	vec4 miscDataC = texelFetch( s_miscData, T, 0 );
	float heat = min( miscDataC.x, 1.0 );
	heat += mouseRatio * u_mouseMoltenHeatStrength;
	miscDataC.x = heat;
	

	// Add molten scalar random value
	miscDataC.y = mix( miscDataC.y, rand(), (mouseRatio * u_mouseMoltenVolumeStrength) > 0.0 ? 1.0 : 0.0 );

	out_miscData = miscDataC;

	// Add slope gradient to flux
	{
		// Dampen
		fC *= u_moltenVelocityDamping;
		
		vec4 hN = texelFetchOffset(s_heightData, T, 0, ivec2( 0, -1));
		vec4 hS = texelFetchOffset(s_heightData, T, 0, ivec2( 0,  1));
		vec4 hE = texelFetchOffset(s_heightData, T, 0, ivec2( 1,  0));
		vec4 hW = texelFetchOffset(s_heightData, T, 0, ivec2(-1,  0));
	
		float mhC = hC.x + hC.y;
		float mhN = hN.x + hN.y;
		float mhS = hS.x + hS.y;
		float mhE = hE.x + hE.y;
		float mhW = hW.x + hW.y;
		
		fC.x += (mhC - mhW) * u_moltenSlopeStrength;
		fC.y += (mhC - mhE) * u_moltenSlopeStrength;
		fC.z += (mhC - mhN) * u_moltenSlopeStrength;
		fC.w += (mhC - mhS) * u_moltenSlopeStrength;
		
		// Slow velocity based on viscosity
		//float moltenViscosity = pow( heat, 0.5 );
		//fluidVelocityC.xy *= min( heat * 3.0, 1.0);

		fC *= smoothstep( 0.0, 0.001, hC.y );
		
		// Limit
		fC = max( fC, vec4(0.0));
	}


	
	out_fluidVelocity = fC;
}