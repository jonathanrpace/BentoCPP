#version 430 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Samplers
uniform sampler2D s_rockData;
uniform sampler2D s_rockFluxData;
uniform sampler2D s_rockNormalData;

uniform sampler2D s_waterData;
uniform sampler2D s_waterFluxData;
uniform sampler2D s_waterNormalData;

uniform sampler2D s_mappingData;
uniform sampler2D s_diffuseMap;

// From VS
in Varying
{
	vec2 in_uv;
};

// Uniforms

// Mouse
uniform vec2 u_mousePos;
uniform float u_mouseRadius;
uniform float u_mouseMoltenVolumeStrength;
uniform float u_mouseWaterVolumeStrength;
uniform float u_mouseMoltenHeatStrength;

// Environment
uniform float u_ambientTemp;

// Molten
uniform float u_heatAdvectSpeed;
uniform float u_viscosityMin;
uniform float u_viscosityMax;
uniform float u_heatViscosityPower;
uniform float u_heatViscosityBias;

uniform float u_velocityScalar;
uniform float u_tempChangeSpeed;
uniform float u_condenseSpeed;
uniform float u_meltSpeed;

// Water
uniform float u_waterViscosity;


// Buffers
layout( std430, binding = 0 ) buffer MousePositionBuffer
{
	int mouseBufferZ;
	int mouseBufferU;
	int mouseBufferV;
};


////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_rockData;
layout( location = 1 ) out vec4 out_waterData;

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

vec2 GetMousePos()
{
	vec2 mousePos = vec2( mouseBufferU, mouseBufferV ) / 255;
	return mousePos;
}

float CalcViscosity( float _heat, float _viscosityScalar )
{
	float viscosity = mix( u_viscosityMin, u_viscosityMax, 1.0f-_viscosityScalar );
	return pow(smoothstep( 0.0f, 1.0f, clamp(_heat-u_heatViscosityBias, 0.0f, 1.0f)), u_heatViscosityPower) * viscosity;
}

vec2 VelocityFromFlux( vec4 fluxC, vec4 fluxL, vec4 fluxR, vec4 fluxU, vec4 fluxD, vec2 texelSize, float viscosity )
{
	vec2 velocity = vec2(	(fluxL.y + fluxC.y) - (fluxR.x + fluxC.x), 
							(fluxU.w + fluxC.w) - (fluxD.z + fluxC.z) );
	velocity /= texelSize;
	velocity *= u_velocityScalar * viscosity;

	return velocity;
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
	ivec2 texelCoordL = texelCoordC - ivec2(1,0);
	ivec2 texelCoordR = texelCoordC + ivec2(1,0);
	ivec2 texelCoordU = texelCoordC - ivec2(0,1);
	ivec2 texelCoordD = texelCoordC + ivec2(0,1);

	// Shared data samples
	vec4 rockDataC = texelFetch(s_rockData, texelCoordC, 0);
	vec4 waterDataC = texelFetch(s_waterData, texelCoordC, 0);
	vec4 mappingDataC = texelFetch(s_mappingData, texelCoordC, 0);
	vec4 diffuseSampleC = texture(s_diffuseMap, in_uv);
	
	// Shared local vars
	float solidHeight = rockDataC.x;
	float dirtHeight = rockDataC.w;
	float moltenHeight = rockDataC.y;
	float moltenHeat = rockDataC.z;
	float iceHeight = waterDataC.x;
	float waterHeight = waterDataC.y;

	float moltenViscosityScalar = mappingDataC.y;
	float moltenViscosity = CalcViscosity(moltenHeat, moltenViscosityScalar);

	vec2 mousePos = GetMousePos();
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-mousePos) / u_mouseRadius);

	////////////////////////////////////////////////////////////////
	// Update molten
	////////////////////////////////////////////////////////////////
	float newMoltenHeight = moltenHeight;
	float newMoltenHeat = moltenHeat;

	{
		vec4 fluxC = texelFetch(s_rockFluxData, texelCoordC, 0);
		vec4 fluxL = texelFetch(s_rockFluxData, texelCoordL, 0);
		vec4 fluxR = texelFetch(s_rockFluxData, texelCoordR, 0);
		vec4 fluxU = texelFetch(s_rockFluxData, texelCoordU, 0);
		vec4 fluxD = texelFetch(s_rockFluxData, texelCoordD, 0);
		vec4 fluxN = vec4(fluxL.y, fluxR.x, fluxU.w, fluxD.z);

		// Update molten height based on flux
		float fluxChange = ((fluxN.x+fluxN.y+fluxN.z+fluxN.w)-(fluxC.x+fluxC.y+fluxC.z+fluxC.w));
		newMoltenHeight = moltenHeight + fluxChange * moltenViscosity;

		// What proportion of our volume did we lose to neighbours?
		// If we lose half our volume, we also lose half our heat.
		float volumeLossProp = ((fluxC.x+fluxC.y+fluxC.z+fluxC.w)* moltenViscosity) / (moltenHeight + 0.001f);
		volumeLossProp = min( 1.0f, volumeLossProp );
		newMoltenHeat -= (volumeLossProp * moltenHeat) * u_heatAdvectSpeed;

		// For each neighbour, determine what proportion of their volume we have gained.
		// We also want to grab the same proportion of their heat.
		// Essentially the inverse of above.
		vec4 rockDataL = texelFetch(s_rockData, texelCoordL, 0);
		vec4 rockDataR = texelFetch(s_rockData, texelCoordR, 0);
		vec4 rockDataU = texelFetch(s_rockData, texelCoordU, 0);
		vec4 rockDataD = texelFetch(s_rockData, texelCoordD, 0);
		vec4 mappingDataL = texelFetch(s_mappingData, texelCoordL, 0);
		vec4 mappingDataR = texelFetch(s_mappingData, texelCoordR, 0);
		vec4 mappingDataU = texelFetch(s_mappingData, texelCoordU, 0);
		vec4 mappingDataD = texelFetch(s_mappingData, texelCoordD, 0);

		vec4 neighbourHeat = vec4(rockDataL.z, rockDataR.z, rockDataU.z, rockDataD.z);
		vec4 neighbourHeight = vec4(rockDataL.y, rockDataR.y, rockDataU.y, rockDataD.y);
		vec4 neighbourViscosity = vec4( 
			CalcViscosity(neighbourHeat.x, mappingDataL.y), 
			CalcViscosity(neighbourHeat.y, mappingDataR.y), 
			CalcViscosity(neighbourHeat.z, mappingDataU.y), 
			CalcViscosity(neighbourHeat.w, mappingDataD.y) 
		);
		vec4 volumeGainProp = (fluxN * neighbourViscosity) / (neighbourHeight + EPSILON);
		volumeGainProp = min( vec4(1.0f), volumeGainProp );
		vec4 heatGain = volumeGainProp * neighbourHeat;
		newMoltenHeat += (heatGain.x + heatGain.y + heatGain.z + heatGain.w) * u_heatAdvectSpeed;

		// Cooling
		// Occluded areas cool slower
		vec4 rockNormalDataC = texelFetch(s_rockNormalData, texelCoordC, 0);
		float occlusion = rockNormalDataC.w;
		newMoltenHeat += (u_ambientTemp - newMoltenHeat) * u_tempChangeSpeed * (1.0f-occlusion);

		// Add some lava near the mouse
		float mouseTextureScalar = diffuseSampleC.x;
		newMoltenHeat	+= ( pow(mouseRatio, 0.5f) * u_mouseMoltenHeatStrength   * mix(0.5f, 1.0f, mouseTextureScalar) ) / (1.0001f+moltenHeat*5.0f);
		newMoltenHeight += ( pow(mouseRatio, 1.5f) * u_mouseMoltenVolumeStrength * mix(0.5f, 1.0f, mouseTextureScalar) ) / (1.0001f+newMoltenHeight);
	}

	////////////////////////////////////////////////////////////////
	// Update water
	////////////////////////////////////////////////////////////////
	float newWaterHeight = waterHeight;

	{
		vec4 fluxC = texelFetch(s_waterFluxData, texelCoordC, 0);
		vec4 fluxL = texelFetch(s_waterFluxData, texelCoordL, 0);
		vec4 fluxR = texelFetch(s_waterFluxData, texelCoordR, 0);
		vec4 fluxU = texelFetch(s_waterFluxData, texelCoordU, 0);
		vec4 fluxD = texelFetch(s_waterFluxData, texelCoordD, 0);
		vec4 fluxN = vec4(fluxL.y, fluxR.x, fluxU.w, fluxD.z);

		// Update water height based on flux
		float fluxChange = ((fluxN.x+fluxN.y+fluxN.z+fluxN.w)-(fluxC.x+fluxC.y+fluxC.z+fluxC.w));
		newWaterHeight = waterHeight + fluxChange * u_waterViscosity;

		// Add some water near the mouse
		float mouseTextureScalar = diffuseSampleC.x;
		newWaterHeight += ( pow(mouseRatio, 1.5f) * u_mouseWaterVolumeStrength * mix(0.5f, 1.0f, mouseTextureScalar) ) / (1.0001f+newWaterHeight);
	}

	////////////////////////////////////////////////////////////////
	// Exchange heat/volume between layers
	////////////////////////////////////////////////////////////////
	{
		float moltenToSolid = 0.0f;
		if ( newMoltenHeat < u_heatViscosityBias )
		{
			moltenToSolid += min( moltenHeight, u_condenseSpeed * moltenHeight );
		}

		float meltStrength = max(newMoltenHeat-u_heatViscosityBias, 0.0f) * u_meltSpeed;// * length(moltenVelocity);
		float solidToMolten = min(solidHeight, meltStrength);

		float solidToMoltenRatio = solidToMolten / (newMoltenHeight + 0.0001f);
		solidToMoltenRatio = clamp(solidToMoltenRatio, 0.0f, 1.0f);

		newMoltenHeat *= (1.0f-solidToMoltenRatio);
		solidHeight += moltenToSolid - solidToMolten;
		newMoltenHeight  += solidToMolten - moltenToSolid;
	}
	
	////////////////////////////////////////////////////////////////
	// Output
	////////////////////////////////////////////////////////////////
	out_rockData = vec4(solidHeight, newMoltenHeight, newMoltenHeat, 0.0f);
	out_waterData = vec4(0.0f, newWaterHeight, 0.0f, 0.0f);
}








