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

in Varying
{
	vec2 in_uv;
};

// Uniforms
uniform vec2 u_mousePos;
uniform float u_viscocity;
uniform float u_mouseRadius;
uniform float u_mouseVolumeStrength;
uniform float u_mouseHeatStrength;
uniform float u_heatAdvectSpeed;

uniform float u_viscosityMin;
uniform float u_viscosityMax;
uniform float u_heatViscosityPower;
uniform float u_heatViscosityBias;

uniform float u_coolingSpeedMin;
uniform float u_coolingSpeedMax;
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
	float viscosity = mix( u_viscosityMin, u_viscosityMax, _viscosityScalar );
	return pow(smoothstep( 0.0f, 1.0f, clamp(_heat-u_heatViscosityBias, 0.0f, 1.0f)), u_heatViscosityPower) * viscosity;
}

void meltCondense(float heat, float moltenHeight, float solidHeight, out float o_moltenHeight, out float o_solidHeight)
{
	float moltenToSolid = 0.0f;
	if ( heat < u_heatViscosityBias )
	{
		moltenToSolid += min( moltenHeight, u_condenseSpeed * moltenHeight );
	}

	float meltStrength = max(heat-u_heatViscosityBias, 0.0f);
	float solidToMolten = min(solidHeight, meltStrength * u_meltSpeed);

	o_solidHeight = solidHeight + moltenToSolid - solidToMolten;
	o_moltenHeight = moltenHeight + solidToMolten - moltenToSolid;
}

void main(void)
{ 
	ivec2 dimensions = textureSize( s_heightData, 0 );
	vec2 texelSize = 1.0f / dimensions;

	vec4 heightDataSample = texture(s_heightData, in_uv);
	vec4 heightDataSampleL = texture(s_heightData, in_uv - vec2(texelSize.x,0.0f));
	vec4 heightDataSampleR = texture(s_heightData, in_uv + vec2(texelSize.x,0.0f));
	vec4 heightDataSampleU = texture(s_heightData, in_uv - vec2(0.0f,texelSize.y));
	vec4 heightDataSampleD = texture(s_heightData, in_uv + vec2(0.0f,texelSize.y));

	vec4 velocityDataSample = texture(s_velocityData, in_uv);

	vec4 mappingData = texture(s_mappingData, in_uv);
	vec4 mappingL = texture(s_mappingData, in_uv - vec2(texelSize.x,0.0f));
	vec4 mappingR = texture(s_mappingData, in_uv + vec2(texelSize.x,0.0f));
	vec4 mappingU = texture(s_mappingData, in_uv - vec2(0.0f,texelSize.y));
	vec4 mappingD = texture(s_mappingData, in_uv + vec2(0.0f,texelSize.y));

	vec4 diffuseSample = texture(s_diffuseMap, mappingData.xy);
	vec4 diffuseSampleL = texture(s_diffuseMap, mappingL.xy);
	vec4 diffuseSampleR = texture(s_diffuseMap, mappingR.xy);
	vec4 diffuseSampleU = texture(s_diffuseMap, mappingU.xy);
	vec4 diffuseSampleD = texture(s_diffuseMap, mappingD.xy);

	float solidHeight = heightDataSample.x;
	float moltenHeight = heightDataSample.y;
	float heat = heightDataSample.z;
	float newHeat = heat;
	vec2 velocity = velocityDataSample.xy;

	float viscosity = CalcViscosity(heat, diffuseSample.y);
	
	vec4 flux = texture(s_fluxData, in_uv);
	float fluxL = texture(s_fluxData, in_uv - vec2(texelSize.x,0.0f)).y;
	float fluxR = texture(s_fluxData, in_uv + vec2(texelSize.x,0.0f)).x;
	float fluxU = texture(s_fluxData, in_uv - vec2(0.0f,texelSize.y)).w;
	float fluxD = texture(s_fluxData, in_uv + vec2(0.0f,texelSize.y)).z;
	vec4 nFlux = vec4(fluxL, fluxR, fluxU, fluxD);

	float fluxChange = ((nFlux.x+nFlux.y+nFlux.z+nFlux.w)-(flux.x+flux.y+flux.z+flux.w));
	float newMoltenHeight = moltenHeight + fluxChange * viscosity;

	vec4 epsilon = vec4(0.001f);

	// What proportion of our volume did we lose to neighbours?
	// If we lose half our volume, we also lose half our heat.
	float volumeLossProp = ((flux.x+flux.y+flux.z+flux.w)* viscosity) / (moltenHeight + 0.001f);
	volumeLossProp = min( 1.0f, volumeLossProp );
	newHeat -= (volumeLossProp * heat) * u_heatAdvectSpeed;

	// For each neighbour, determine what proportion of their volume we have gained.
	// We also want to grab the same proportion of their heat.
	// Essentially the inverse of above.
	vec4 neighbourHeat = vec4(heightDataSampleL.z, heightDataSampleR.z, heightDataSampleU.z, heightDataSampleD.z);
	vec4 neighbourHeight = vec4(heightDataSampleL.y, heightDataSampleR.y, heightDataSampleU.y, heightDataSampleD.y);
	vec4 neighbourViscosity = vec4( 
		CalcViscosity(neighbourHeat.x, diffuseSampleL.y), 
		CalcViscosity(neighbourHeat.y, diffuseSampleR.y), 
		CalcViscosity(neighbourHeat.z, diffuseSampleU.y), 
		CalcViscosity(neighbourHeat.w, diffuseSampleD.y) 
	);

	vec4 volumeGainProp = (nFlux * neighbourViscosity) / (neighbourHeight + epsilon);
	volumeGainProp = min( vec4(1.0f), volumeGainProp );
	vec4 heatGain = volumeGainProp * neighbourHeat;
	newHeat += (heatGain.x + heatGain.y + heatGain.z + heatGain.w) * u_heatAdvectSpeed;

	// Cooling
	// Higher bits cool faster
	float coolingSpeed = diffuseSample.y * u_coolingSpeedMax + (1.0f - diffuseSample.x) * u_coolingSpeedMin;
	newHeat = max(0.0f, newHeat - coolingSpeed);

	// Add some lava near the mouse

	//Heat up the land - introduce molten height from zero (need to count via uniform), only add molten height when uniform is > solid height.
	//Should look like lava is rising

	vec2 mousePos = GetMousePos();
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-mousePos) / u_mouseRadius);
	mouseRatio = pow(mouseRatio, 1.5f);
	newHeat += (mouseRatio*u_mouseHeatStrength) / (1.0001f+heat);
	newMoltenHeight += (mouseRatio * u_mouseVolumeStrength) / (1.0001f+newMoltenHeight);
	

	float outMoltenHeight = newMoltenHeight;
	float outSolidHeight = solidHeight;
	meltCondense(newHeat, newMoltenHeight, solidHeight, outMoltenHeight, outSolidHeight);
	
	out_heightData = vec4(outSolidHeight, outMoltenHeight, newHeat, heightDataSample.w);
}








