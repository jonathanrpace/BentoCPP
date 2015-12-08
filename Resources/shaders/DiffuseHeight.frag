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
uniform float u_meltSpeed;
uniform float u_condenseSpeed;

// Outputs
layout( location = 0 ) out vec4 out_heightData;

void meltCondense(float heat, float moltenHeight, float solidHeight, out float o_moltenHeight, out float o_solidHeight)
{
	o_moltenHeight = moltenHeight;
	o_solidHeight = solidHeight;

	float moltenToSolid = 0.0f;
	if ( heat < u_heatViscosityBias )
	{
		moltenToSolid += min( moltenHeight, u_condenseSpeed );
	}

	float clampedHeat = clamp(heat-u_heatViscosityBias, 0.0f, 1.0f);
	float meltStrength = clampedHeat;
	float solidToMolten = min(solidHeight+moltenToSolid, meltStrength * u_meltSpeed);

	o_solidHeight -= solidToMolten;
	o_solidHeight += moltenToSolid;

	o_moltenHeight -= moltenToSolid;
	o_moltenHeight += solidToMolten;
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

	

	// Smooth molten height
	newMoltenHeight += clamp((heightA.y-height.y), -heightA.y*0.5, heightA.y*0.5) * u_strength * 0.5f;
	newMoltenHeight += clamp((heightB.y-height.y), -heightB.y*0.5, heightB.y*0.5) * u_strength * 0.5f;

	meltCondense(newHeat, newMoltenHeight, height.x, newMoltenHeight, newSolidHeight);
	
	out_heightData = vec4(newSolidHeight, newMoltenHeight, newHeat, height.w);
}





