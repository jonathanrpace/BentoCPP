#version 430 core

const vec4 EPSILON = vec4(0.000001f);
const float PI = 3.14159265359;
const float HALF_PI = PI * 0.5f;

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// From VS
in Varying
{
	vec2 in_uv;
};

// Uniforms

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_moltenFluxData;
uniform sampler2D s_waterFluxData;
uniform sampler2D s_miscData;
uniform sampler2D s_pressureData;

// Molten
uniform vec2 u_moltenViscosity;
uniform vec2 u_moltenDamping;
uniform float u_moltenMinHeat;
uniform float u_moltenPressureScale;
uniform float u_moltenSlopeStrength;

// Water
uniform float u_waterViscosity;
uniform float u_waterSlopeStrength;
//uniform float u_waterFluxDamping;

const float DT = 1.0 / 6.0;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_moltenFluxData;
layout( location = 1 ) out vec4 out_waterFluxData;

////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////
void main(void)
{ 
	ivec2 T = ivec2(gl_FragCoord.xy);
	
	vec4 hC = texelFetch(s_heightData, T, 0);
	vec4 hN = texelFetchOffset(s_heightData, T, 0, ivec2( 0,-1));
	vec4 hS = texelFetchOffset(s_heightData, T, 0, ivec2( 0, 1));
	vec4 hE = texelFetchOffset(s_heightData, T, 0, ivec2( 1, 0));
	vec4 hW = texelFetchOffset(s_heightData, T, 0, ivec2(-1, 0));
	float pressureRatioC = clamp( texelFetchOffset(s_pressureData, T, 0, ivec2( 0,  0)).x * 10000.0, 0.0, 1.0 );

	vec4 moltenFluxC = texelFetch(s_moltenFluxData, T, 0);
	vec4 waterFluxC = texelFetch(s_waterFluxData, T, 0);
	
	// Update molten flux
	{
		vec4 miscData = texelFetch(s_miscData, T, 0);

		float heatRatio = max( (miscData.x + pressureRatioC) - u_moltenMinHeat, 0.0 ) / ( 1.0 - u_moltenMinHeat );

		float moltenViscosity = mix( u_moltenViscosity.x, u_moltenViscosity.y, heatRatio );

		// Add slope
		float mhC = hC.x + hC.y;
		float mhN = hN.x + hN.y;
		float mhS = hS.x + hS.y;
		float mhE = hE.x + hE.y;
		float mhW = hW.x + hW.y;
		vec4 diffs = vec4( mhC - mhW, mhC - mhE, mhC - mhN, mhC - mhS );
		diffs *= u_moltenSlopeStrength;

		/* Advect neighbour flux to this cell */
		{
			vec4 fluxC = texelFetch(s_moltenFluxData, T, 0);
			vec4 fluxN = texelFetchOffset(s_moltenFluxData, T, 0, ivec2( 0,-1));
			vec4 fluxS = texelFetchOffset(s_moltenFluxData, T, 0, ivec2( 0, 1));
			vec4 fluxE = texelFetchOffset(s_moltenFluxData, T, 0, ivec2( 1, 0));
			vec4 fluxW = texelFetchOffset(s_moltenFluxData, T, 0, ivec2(-1, 0));
		
			float toN = min( fluxC.z, hC.y ) * DT;
			float toS = min( fluxC.w, hC.y ) * DT;
			float toE = min( fluxC.y, hC.y ) * DT;
			float toW = min( fluxC.x, hC.y ) * DT;
			float totalTo = (toN + toS + toE + toW);

			float propC = hC.y > 0.002 ? min( 1.0, totalTo / hC.y ) : 0.0;

			float advectSpeed = 0.99;
			moltenFluxC -= propC * moltenFluxC * advectSpeed;

			float fromN = min( fluxN.w, hN.y ) * DT;
			float fromS = min( fluxS.z, hS.y ) * DT;
			float fromE = min( fluxE.x, hE.y ) * DT;
			float fromW = min( fluxW.y, hW.y ) * DT;

			float propN = hN.y > 0.002 ? min( 1.0, fromN / hN.y ) : 0.0;
			float propS = hS.y > 0.002 ? min( 1.0, fromS / hS.y ) : 0.0;
			float propE = hE.y > 0.002 ? min( 1.0, fromE / hE.y ) : 0.0;
			float propW = hW.y > 0.002 ? min( 1.0, fromW / hW.y ) : 0.0;

			moltenFluxC += (propN * fluxN) * advectSpeed;
			moltenFluxC += (propS * fluxS) * advectSpeed;
			moltenFluxC += (propE * fluxE) * advectSpeed;
			moltenFluxC += (propW * fluxW) * advectSpeed;
		}
		
		
		// Add pressure gradient
		/*
		{
			float pC = texelFetchOffset(s_pressureData, T, 0, ivec2( 0,  0)).x * 10000.0;
			float pN = texelFetchOffset(s_pressureData, T, 0, ivec2( 0, -1)).x * 10000.0;
			float pS = texelFetchOffset(s_pressureData, T, 0, ivec2( 0,  1)).x * 10000.0;
			float pE = texelFetchOffset(s_pressureData, T, 0, ivec2( 1,  0)).x * 10000.0;
			float pW = texelFetchOffset(s_pressureData, T, 0, ivec2(-1,  0)).x * 10000.0;
			
			pC = max( 0.0, pC );
			pN = max( 0.0, pN );
			pS = max( 0.0, pS );
			pE = max( 0.0, pE );
			pW = max( 0.0, pW );
		
			diffs.x += (pC - pW) * u_moltenPressureScale;
			diffs.y += (pC - pE) * u_moltenPressureScale;
			diffs.z += (pC - pN) * u_moltenPressureScale;
			diffs.w += (pC - pS) * u_moltenPressureScale;
		}
		*/

		moltenFluxC = max( vec4(0.0), moltenFluxC + diffs * DT * moltenViscosity );
		
		// Limit the change so we're not draining more fluid than we have
		float scalingFactor = min( 1.0, mhC / (moltenFluxC.x + moltenFluxC.y + moltenFluxC.z + moltenFluxC.w) * 1.0 );
		moltenFluxC *= scalingFactor;
		
		// Damping
		{
			float damping = mix( u_moltenDamping.x, u_moltenDamping.y, heatRatio );
			moltenFluxC *= damping;

			// Dampen to zero when no volume
			float volumeRatio = min( 1.0, hC.y / 0.0001 );
			moltenFluxC *= volumeRatio;
		}
	}
	
	
	// Udpate water flux
	{
		// Add slope
		float mhC = hC.x + hC.y + hC.z + hC.w;
		float mhN = hN.x + hN.y + hN.z + hN.w;
		float mhS = hS.x + hS.y + hS.z + hS.w;
		float mhE = hE.x + hE.y + hE.z + hE.w;
		float mhW = hW.x + hW.y + hW.z + hW.w;
		vec4 diffs = vec4( mhC - mhW, mhC - mhE, mhC - mhN, mhC - mhS ) * u_waterSlopeStrength;
		
		waterFluxC = max( vec4(0.0), waterFluxC + diffs * DT );
		
		// Limit the change so we're not draining more fluid than we have
		float scalingFactor = min( 1.0, hC.w / (waterFluxC.x + waterFluxC.y + waterFluxC.z + waterFluxC.w) );
		waterFluxC *= scalingFactor;
		
		// Damping
		{
			// Dampen to zero when no volume
			float volumeRatio = min( 1.0, hC.w / 0.001 );
			waterFluxC *= volumeRatio;
			
			// Damping based on viscosity
			waterFluxC *= u_waterViscosity;
		}
	}
	
	out_moltenFluxData = moltenFluxC;
	out_waterFluxData = waterFluxC;
}