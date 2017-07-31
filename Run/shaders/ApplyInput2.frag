#version 430 core

// From VS
in Varying
{
	vec2 in_uv;
};

void srand(float seed);
float rand();

//layout( location = 0 ) out vec4 out_heightData;
layout( location = 0 ) out vec4 out_fluidVelocity;
//layout( location = 2 ) out vec4 out_miscData;

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

	vec4 fC = texelFetch( s_fluidVelocity, T, 0 ); 
	out_fluidVelocity = fC;
	
	
	
	// Add heat
	/*
	vec4 miscDataC = texelFetch( s_miscData, T, 0 );
	float heat = min( miscDataC.x, 1.0 );
	heat += mouseRatio * u_mouseMoltenHeatStrength;
	miscDataC.x = heat;
	out_miscData = miscDataC;
	*/
	
	// Modify flux
	if ( true )
	{
		
	}


	
	out_fluidVelocity = fC;
}