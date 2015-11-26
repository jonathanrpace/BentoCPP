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
uniform float u_heatViscosityPower;
uniform float u_coolingSpeed;
uniform float u_meltCondensePower;
uniform float u_meltCondenseSpeed;

// Outputs
layout( location = 0 ) out vec4 out_heightData;

void main(void)
{ 
	ivec2 dimensions = textureSize( s_heightData, 0 );
	vec2 texelSize = 1.0f / dimensions;

	vec4 heightDataSample = texture(s_heightData, in_uv);
	vec4 flux = texture(s_fluxData, in_uv);
	//vec4 velocityDataSample = texture(s_velocityData, in_uv);

	float solidHeight = heightDataSample.x;
	float moltenHeight = heightDataSample.y;
	float heat = heightDataSample.z;

	float viscosity = pow(heat, u_heatViscosityPower) * u_viscocity;
	float newHeat = max(0.0f, heat - u_coolingSpeed);

	float fluxL = texture(s_fluxData, in_uv - vec2(texelSize.x,0.0f)).y;
	float fluxR = texture(s_fluxData, in_uv + vec2(texelSize.x,0.0f)).x;
	float fluxU = texture(s_fluxData, in_uv - vec2(0.0f,texelSize.y)).w;
	float fluxD = texture(s_fluxData, in_uv + vec2(0.0f,texelSize.y)).z;
	vec4 nFlux = vec4(fluxL, fluxR, fluxU, fluxD);

	float fluxChange = ((nFlux.x+nFlux.y+nFlux.z+nFlux.w)-(flux.x+flux.y+flux.z+flux.w)) * viscosity;
	float newMoltenHeight = moltenHeight + fluxChange;

	// Add some lava near the mouse
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-u_mousePos) / u_mouseRadius);
	mouseRatio = pow(mouseRatio, 1.5f);
	newMoltenHeight += mouseRatio * u_mouseStrength;
	newHeat = min( 1.0f, newHeat + ((mouseRatio*u_mouseStrength) > 0.0 ? 0.01f : 0.0f) );
	newHeat = clamp( newHeat, 0.0f, 1.0f );

	
	
	out_heightData = vec4(solidHeight, newMoltenHeight, newHeat, heightDataSample.w);
}








