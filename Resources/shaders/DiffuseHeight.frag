#version 330 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_velocityData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform vec2 u_axis;
uniform float u_strength;
uniform float u_heatDissipation;
uniform float u_heatViscosity;
uniform float u_coolingSpeed;
uniform float u_meltCondensePower;
uniform float u_meltCondenseSpeed;

// Outputs
layout( location = 0 ) out vec4 out_heightData;

void meltCondense(float heat, float moltenHeight, float solidHeight, out float o_moltenHeight, out float o_solidHeight)
{
	o_moltenHeight = moltenHeight;
	o_solidHeight = solidHeight;

	float clampedHeat = clamp(heat, 0.0f, 1.0f);

	float meltStrength = pow(clampedHeat, u_meltCondensePower);
	float solidToMolten = min(solidHeight * u_meltCondenseSpeed, meltStrength * u_meltCondenseSpeed);
	o_solidHeight -= solidToMolten;
	o_moltenHeight += solidToMolten;

	// Or condense molten to rock
	float condenseStrength = pow(1.0f-clampedHeat, u_meltCondensePower);
	float moltenToSolid = min(moltenHeight * u_meltCondenseSpeed, condenseStrength * u_meltCondenseSpeed);
	o_solidHeight += moltenToSolid;
	o_moltenHeight -= moltenToSolid;
}


void main(void)
{ 
	ivec2 dimensions = textureSize(s_heightData, 0);
	vec2 texelSize = 1.0f / dimensions;

	vec4 height = texture(s_heightData, in_uv);
	vec4 heightA = texture(s_heightData, in_uv - u_axis * texelSize);
	vec4 heightB = texture(s_heightData, in_uv + u_axis * texelSize);

	float heat = height.z;
	float solidHeight = height.x;
	float newMoltenHeight = height.y;
	
	meltCondense(heat, height.y, height.x, newMoltenHeight, solidHeight);

	// Smooth molten height
	//newMoltenHeight += clamp((heightA.y-height.y), -heightA.y*0.5, heightA.y*0.5) * u_strength * 0.5f;
	//newMoltenHeight += clamp((heightB.y-height.y), -heightB.y*0.5, heightB.y*0.5) * u_strength * 0.5f;
	
	// Move heat
	vec2 velocity = texture(s_velocityData, in_uv).xy;
	vec2 velocityA = texture(s_velocityData, in_uv - u_axis * texelSize).xy;
	vec2 velocityB = texture(s_velocityData, in_uv + u_axis * texelSize).xy;

	float newHeat = heat;
	vec2 fromA = max(vec2(0.0f), velocityA) * u_axis * heightA.z * 0.5f;
	vec2 fromB = max(vec2(0.0f),-velocityB) * u_axis * heightB.z * 0.5f;
	newHeat += (fromA.x + fromA.y + fromB.x + fromB.y) * u_heatViscosity;
	//heat = clamp(newHeat, 0.0f, 1.0f+newMoltenHeight);

	vec2 toA = max(vec2(0.0f), velocity) * u_axis * heat * 0.5f;
	toA = min(toA, 1.0f+heightA.y);
	vec2 toB = max(vec2(0.0f),-velocity) * u_axis * heat * 0.5f;
	toB = min(toB, 1.0f+heightA.y);
	newHeat -= (toA.x+toA.y+toB.x+toB.y) * u_heatViscosity;

	heat = clamp(newHeat, 0.0f, 1.0f+newMoltenHeight);
	
	// Smooth heat
	heat += clamp((heightA.z-height.z), -heightA.z*0.25, heightA.z*0.25) * u_heatDissipation;
	heat += clamp((heightB.z-height.z), -heightB.z*0.25, heightB.z*0.25) * u_heatDissipation;

	heat = clamp(heat, 0.0f, 1.0f+newMoltenHeight);

	out_heightData = vec4(solidHeight, newMoltenHeight, heat, height.w);
}





