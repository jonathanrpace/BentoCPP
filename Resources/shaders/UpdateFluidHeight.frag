#version 430 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_fluxData;
uniform sampler2D s_velocityData;
uniform sampler2D s_diffuseMap;
uniform sampler2D s_mappingData;
uniform sampler2D s_normalData;

in Varying
{
	vec2 in_uv;
};

// Uniforms
uniform vec2 u_mousePos;
uniform float u_viscocity;
uniform float u_heatAdvectSpeed;

uniform float u_mouseRadius;
uniform float u_mouseVolumeStrength;
uniform float u_mouseHeatStrength;

uniform float u_viscosityMin;
uniform float u_viscosityMax;
uniform float u_heatViscosityPower;
uniform float u_heatViscosityBias;

uniform float u_tempChangeSpeed;
uniform float u_ambientTemp;
uniform float u_condenseSpeed;
uniform float u_meltSpeed;

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

layout( location = 0 ) out vec4 out_heightData;

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

void meltCondense(float heat, float moltenHeight, float solidHeight, vec2 velocity, out float o_moltenHeight, out float o_solidHeight, out float o_heat)
{
	float moltenToSolid = 0.0f;
	if ( heat < u_heatViscosityBias )
	{
		moltenToSolid += min( moltenHeight, u_condenseSpeed * moltenHeight );
	}

	float meltStrength = max(heat-u_heatViscosityBias, 0.0f) * u_meltSpeed * length(velocity);
	float solidToMolten = min(solidHeight, meltStrength);

	o_solidHeight = solidHeight + moltenToSolid - solidToMolten;
	o_moltenHeight = moltenHeight + solidToMolten - moltenToSolid;

	float solidToMoltenRatio = solidToMolten / (moltenHeight + 0.0001f);
	solidToMoltenRatio = clamp(solidToMoltenRatio, 0.0f, 1.0f);

	o_heat = heat * (1.0f-solidToMoltenRatio);
}

void main(void)
{ 
	//ivec2 dimensions = textureSize( s_heightData, 0 );
	//vec2 texelSize = 1.0f / dimensions;

	ivec2 texelCoordC = ivec2(gl_FragCoord.xy);
	ivec2 texelCoordL = texelCoordC - ivec2(1,0);
	ivec2 texelCoordR = texelCoordC + ivec2(1,0);
	ivec2 texelCoordU = texelCoordC - ivec2(0,1);
	ivec2 texelCoordD = texelCoordC + ivec2(0,1);

	vec4 heightDataC = texelFetch(s_heightData, texelCoordC, 0);
	vec4 heightDataL = texelFetch(s_heightData, texelCoordL, 0);
	vec4 heightDataR = texelFetch(s_heightData, texelCoordR, 0);
	vec4 heightDataU = texelFetch(s_heightData, texelCoordU, 0);
	vec4 heightDataD = texelFetch(s_heightData, texelCoordD, 0);

	vec4 mappingDataC = texelFetch(s_mappingData, texelCoordC, 0);
	vec4 mappingDataL = texelFetch(s_mappingData, texelCoordL, 0);
	vec4 mappingDataR = texelFetch(s_mappingData, texelCoordR, 0);
	vec4 mappingDataU = texelFetch(s_mappingData, texelCoordU, 0);
	vec4 mappingDataD = texelFetch(s_mappingData, texelCoordD, 0);

	vec4  fluxC = texelFetch(s_fluxData, texelCoordC, 0);
	float fluxL = texelFetch(s_fluxData, texelCoordL, 0).y;
	float fluxR = texelFetch(s_fluxData, texelCoordR, 0).x;
	float fluxU = texelFetch(s_fluxData, texelCoordU, 0).w;
	float fluxD = texelFetch(s_fluxData, texelCoordD, 0).z;

	vec4 diffuseSampleC = texture(s_diffuseMap, in_uv);
	//vec4 diffuseSampleL = texture(s_diffuseMap, mappingDataL.xy);
	//vec4 diffuseSampleR = texture(s_diffuseMap, mappingDataR.xy);
	//vec4 diffuseSampleU = texture(s_diffuseMap, mappingDataU.xy);
	//vec4 diffuseSampleD = texture(s_diffuseMap, mappingDataD.xy);

	vec4 velocityDataC = texelFetch(s_velocityData, texelCoordC, 0);
	vec4 normalDataC = texelFetch(s_normalData, texelCoordC, 0);

	float solidHeight = heightDataC.x;
	float moltenHeight = heightDataC.y;
	float heat = heightDataC.z;
	float newHeat = heat;
	vec2 velocity = velocityDataC.xy;
	float occlusion = normalDataC.w;
	vec4 nFlux = vec4(fluxL, fluxR, fluxU, fluxD);
	float viscosity = CalcViscosity(heat, mappingDataC.y);

	float fluxChange = ((nFlux.x+nFlux.y+nFlux.z+nFlux.w)-(fluxC.x+fluxC.y+fluxC.z+fluxC.w));
	float newMoltenHeight = moltenHeight + fluxChange * viscosity;

	const vec4 epsilon = vec4(0.001f);

	// What proportion of our volume did we lose to neighbours?
	// If we lose half our volume, we also lose half our heat.
	float volumeLossProp = ((fluxC.x+fluxC.y+fluxC.z+fluxC.w)* viscosity) / (moltenHeight + 0.001f);
	volumeLossProp = min( 1.0f, volumeLossProp );
	newHeat -= (volumeLossProp * heat) * u_heatAdvectSpeed;

	// For each neighbour, determine what proportion of their volume we have gained.
	// We also want to grab the same proportion of their heat.
	// Essentially the inverse of above.
	vec4 neighbourHeat = vec4(heightDataL.z, heightDataR.z, heightDataU.z, heightDataD.z);
	vec4 neighbourHeight = vec4(heightDataL.y, heightDataR.y, heightDataU.y, heightDataD.y);
	vec4 neighbourViscosity = vec4( 
		CalcViscosity(neighbourHeat.x, mappingDataL.y), 
		CalcViscosity(neighbourHeat.y, mappingDataR.y), 
		CalcViscosity(neighbourHeat.z, mappingDataU.y), 
		CalcViscosity(neighbourHeat.w, mappingDataD.y) 
	);
	vec4 volumeGainProp = (nFlux * neighbourViscosity) / (neighbourHeight + epsilon);
	volumeGainProp = min( vec4(1.0f), volumeGainProp );
	vec4 heatGain = volumeGainProp * neighbourHeat;
	newHeat += (heatGain.x + heatGain.y + heatGain.z + heatGain.w) * u_heatAdvectSpeed;

	// Cooling
	// Occluded areas cool slower
	newHeat += (u_ambientTemp - newHeat) * u_tempChangeSpeed * (1.0f-occlusion);
	
	// Add some lava near the mouse
	vec2 mousePos = GetMousePos();
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-mousePos) / u_mouseRadius);
	newHeat			+= (pow(mouseRatio, 0.5f) * u_mouseHeatStrength   * (1.0f + diffuseSampleC.y*0.0f) ) / (1.0001f+heat);
	newMoltenHeight += (pow(mouseRatio, 1.5f) * u_mouseVolumeStrength * (0.99f + diffuseSampleC.y*0.01f) ) / (1.0001f+newMoltenHeight);
	
	// Melt/Condense
	float outMoltenHeight = newMoltenHeight;
	float outSolidHeight = solidHeight;
	float outHeat = newHeat;
	meltCondense(newHeat, newMoltenHeight, solidHeight, velocity, outMoltenHeight, outSolidHeight, outHeat);
	
	// Out
	out_heightData = vec4(outSolidHeight, outMoltenHeight, outHeat, heightDataC.w);
}








