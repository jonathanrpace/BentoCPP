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
uniform float u_heatViscosityBias;
uniform float u_coolingSpeed;
uniform float u_meltPower;
uniform float u_meltSpeed;
uniform float u_condensePower;
uniform float u_condenseSpeed;

// Outputs
layout( location = 0 ) out vec4 out_heightData;

void meltCondense(float heat, float moltenHeight, float solidHeight, out float o_moltenHeight, out float o_solidHeight)
{
	o_moltenHeight = moltenHeight;
	o_solidHeight = solidHeight;

	float clampedHeat = clamp(heat-u_heatViscosityBias, 0.0f, 1.0f);

	//float meltStrength = pow(clampedHeat, u_meltPower);
	float condenseStrength = pow(1.0f-clampedHeat, u_condensePower);
	float meltStrength = 1.0f-condenseStrength;
	//meltStrength = meltStrength > condenseStrength ? meltStrength : 0.0f;
	//condenseStrength = condenseStrength > meltStrength ? condenseStrength : 0.0f;


	float solidToMolten = min(solidHeight * u_meltSpeed, meltStrength * u_meltSpeed);
	o_solidHeight -= solidToMolten;
	o_moltenHeight += solidToMolten;

	// Or condense molten to rock
	float moltenToSolid = min(moltenHeight * u_condenseSpeed, condenseStrength * u_condenseSpeed);
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
	float newHeat = height.z;
	float newSolidHeight = height.x;
	float newMoltenHeight = height.y;
	
	// Smooth heat
	//newHeat += clamp((heightA.z-heat), -heightA.z*0.25, heightA.z*0.25) * u_heatDissipation;
	//newHeat += clamp((heightB.z-heat), -heightB.z*0.25, heightB.z*0.25) * u_heatDissipation;

	meltCondense(newHeat, height.y, height.x, newMoltenHeight, newSolidHeight);

	// Smooth molten height
	//newMoltenHeight += clamp((heightA.y-height.y), -heightA.y*0.5, heightA.y*0.5) * u_strength * 0.5f;
	//newMoltenHeight += clamp((heightB.y-height.y), -heightB.y*0.5, heightB.y*0.5) * u_strength * 0.5f;
	
	out_heightData = vec4(newSolidHeight, newMoltenHeight, newHeat, height.w);
}





