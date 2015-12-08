#version 330 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_fluxData;
uniform sampler2D s_velocityData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform vec2 u_mousePos;
uniform float u_viscocity;
uniform float u_mouseRadius;
uniform float u_mouseStrength;
uniform float u_heatAdvectSpeed;
uniform float u_viscosity;
uniform float u_heatViscosityPower;
uniform float u_heatViscosityBias;
uniform float u_coolingSpeed;

// Outputs
layout( location = 0 ) out vec4 out_heightData;

float CalcViscosity( float _heat )
{
	return pow(clamp(_heat-u_heatViscosityBias, 0.0f, 1.0f), u_heatViscosityPower) * u_viscosity;
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

	float solidHeight = heightDataSample.x;
	float moltenHeight = heightDataSample.y;
	float heat = heightDataSample.z;
	float newHeat = heat;
	vec2 velocity = velocityDataSample.xy;

	float viscosity = CalcViscosity(heat);
	
	vec4 flux = texture(s_fluxData, in_uv);
	float fluxL = texture(s_fluxData, in_uv - vec2(texelSize.x,0.0f)).y;
	float fluxR = texture(s_fluxData, in_uv + vec2(texelSize.x,0.0f)).x;
	float fluxU = texture(s_fluxData, in_uv - vec2(0.0f,texelSize.y)).w;
	float fluxD = texture(s_fluxData, in_uv + vec2(0.0f,texelSize.y)).z;
	vec4 nFlux = vec4(fluxL, fluxR, fluxU, fluxD);

	float fluxChange = ((nFlux.x+nFlux.y+nFlux.z+nFlux.w)-(flux.x+flux.y+flux.z+flux.w));
	float newMoltenHeight = moltenHeight + fluxChange * viscosity;

	// Send heat to neighbours
	vec4 epsilon = vec4(0.001f);
	vec4 heatToNeighbours = (flux / (moltenHeight+epsilon)) * viscosity * heat * u_heatAdvectSpeed;
	heatToNeighbours = clamp(heatToNeighbours, 0.0f, heat);
	newHeat -= (heatToNeighbours.x + heatToNeighbours.y + heatToNeighbours.z + heatToNeighbours.w);

	// Gather heat from neighbours
	vec4 neighbourHeat = vec4(heightDataSampleL.z, heightDataSampleR.z, heightDataSampleU.z, heightDataSampleD.z);
	vec4 neighbourHeight = vec4(heightDataSampleL.y, heightDataSampleR.y, heightDataSampleU.y, heightDataSampleD.y);
	vec4 neighbourViscosity = vec4( CalcViscosity(neighbourHeat.x), CalcViscosity(neighbourHeat.y), CalcViscosity(neighbourHeat.z), CalcViscosity(neighbourHeat.w) );
	vec4 heatFromNeighbours = (nFlux / (neighbourHeight+epsilon)) * neighbourViscosity * neighbourHeat * u_heatAdvectSpeed;
	heatFromNeighbours = clamp( heatFromNeighbours, vec4(0.0f), neighbourHeat );
	newHeat += (heatFromNeighbours.x + heatFromNeighbours.y + heatFromNeighbours.z + heatFromNeighbours.w);

	newHeat = max(0.0f, newHeat - u_coolingSpeed);
	
	// Add some lava near the mouse
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-u_mousePos) / u_mouseRadius);
	newMoltenHeight += pow(mouseRatio, 1.5f) * u_mouseStrength;
	newHeat += mouseRatio*u_mouseStrength > 0.0 ? min(0.25f,  (u_mouseStrength * mouseRatio * 20.0f) / newHeat) : 0.0f;

	
	
	out_heightData = vec4(solidHeight, newMoltenHeight, newHeat, heightDataSample.w);
}








