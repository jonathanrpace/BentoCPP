#version 430 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Samplers
uniform sampler2D s_rockData;
uniform sampler2D s_rockFluxData;

uniform sampler2D s_waterData;
uniform sampler2D s_waterFluxData;

// From VS
in Varying
{
	vec2 in_uv;
};

// Uniforms
uniform ivec2 u_axis;
uniform float u_viscosityMin;
uniform float u_rockMeltingPoint;
uniform float u_waterViscosity;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_rockData;
layout( location = 1 ) out vec4 out_waterData;

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
//
float CalcViscosity( float _heat )
{
	return smoothstep( 0.0f, 1.0f, clamp(_heat-u_rockMeltingPoint, 0.0f, 1.0)) * u_viscosityMin;
}

////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////

void main(void)
{ 
	const vec4 EPSILON = vec4(0.001f);

	ivec2 dimensions = textureSize( s_rockData, 0 );
	vec2 texelSize = 1.0f / dimensions;
	ivec2 texelCoordC = ivec2(gl_FragCoord.xy);
	ivec2 texelCoordL = texelCoordC - ivec2(1,1) * u_axis;
	ivec2 texelCoordR = texelCoordC + ivec2(1,1) * u_axis;

	// Shared data samples
	vec4 rockDataC = texelFetch(s_rockData, texelCoordC, 0);
	vec4 waterDataC = texelFetch(s_waterData, texelCoordC, 0);
	
	// Shared local vars
	float solidHeight = rockDataC.x;
	float dirtHeight = rockDataC.w;
	float moltenHeight = rockDataC.y;
	float moltenHeat = rockDataC.z;
	float waterHeight = waterDataC.x;
	float moltenViscosity = CalcViscosity(moltenHeat);

	////////////////////////////////////////////////////////////////
	// Update molten
	////////////////////////////////////////////////////////////////
	float newMoltenHeight = moltenHeight;
	{
		vec2 fluxC;
		float fluxL;
		float fluxR;
		if ( u_axis == ivec2(1,0) )
		{
			fluxC = texelFetch(s_rockFluxData, texelCoordC, 0).xy;
			fluxL = texelFetch(s_rockFluxData, texelCoordL, 0).y;
			fluxR = texelFetch(s_rockFluxData, texelCoordR, 0).x;
		}
		else
		{
			fluxC = texelFetch(s_rockFluxData, texelCoordC, 0).zw;
			fluxL = texelFetch(s_rockFluxData, texelCoordL, 0).w;
			fluxR = texelFetch(s_rockFluxData, texelCoordR, 0).z;
		}
		
		// Update molten height based on flux
		float fluxChange = (fluxL+fluxR)-(fluxC.x+fluxC.y);
		newMoltenHeight = moltenHeight + fluxChange * moltenViscosity;
	}

	////////////////////////////////////////////////////////////////
	// Update water
	////////////////////////////////////////////////////////////////
	float newWaterHeight = waterHeight;
	{
		vec2 fluxC;
		float fluxL;
		float fluxR;
		if ( u_axis == ivec2(1,0) )
		{
			fluxC = texelFetch(s_waterFluxData, texelCoordC, 0).xy;
			fluxL = texelFetch(s_waterFluxData, texelCoordL, 0).y;
			fluxR = texelFetch(s_waterFluxData, texelCoordR, 0).x;
		}
		else
		{
			fluxC = texelFetch(s_waterFluxData, texelCoordC, 0).zw;
			fluxL = texelFetch(s_waterFluxData, texelCoordL, 0).w;
			fluxR = texelFetch(s_waterFluxData, texelCoordR, 0).z;
		}
		
		// Update water height based on flux
		float fluxChange = (fluxL+fluxR)-(fluxC.x+fluxC.y);
		newWaterHeight += fluxChange * u_waterViscosity;
	}

	////////////////////////////////////////////////////////////////
	// Output
	////////////////////////////////////////////////////////////////
	
	out_rockData = vec4(solidHeight, newMoltenHeight, moltenHeat, 0.0);
	out_waterData = vec4(newWaterHeight, waterDataC.yzw);
}








