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
	
<<<<<<< HEAD

	// Add molten scalar random value
	miscDataC.y = mix( miscDataC.y, rand(), (mouseRatio * u_mouseMoltenVolumeStrength) > 0.0 ? 1.0 : 0.0 );

	out_miscData = miscDataC;
<<<<<<< HEAD
	*/
	
	// Modify flux
	if ( true )
=======
=======

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
>>>>>>> 6990942d30429b7816ece32f62c4e5cbecfa32f2

	// Add slope gradient to flux
>>>>>>> 6990942d30429b7816ece32f62c4e5cbecfa32f2
	{
		
	}


	
	out_fluidVelocity = fC;
}