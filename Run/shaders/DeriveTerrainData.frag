#version 330 core

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

// Misc
uniform vec2 u_cellSize;
uniform int u_numHeightMips;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_normalData;
layout( location = 1 ) out vec4 out_derivedData;

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

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
	
	out_normalData = vec4(0.0);
	out_derivedData = vec4(0.0);
	
	////////////////////////////////////////////////////////////////
	// Spawn Foam
	////////////////////////////////////////////////////////////////
	/*
	{
		vec2 waterVelocity = out_velocityData.zw;

		float foamAmount = smudgeDataC.w;
		foamAmount *= u_foamDecayRate;
		foamAmount += length( waterVelocity ) * u_foamSpawnStrength;
		foamAmount = min(1.0, foamAmount);

		out_smudgeData.w = foamAmount;
	}
	*/
	
	//////////////////////////////////////////////////////////////////////////////////
	// Solid normal
	//////////////////////////////////////////////////////////////////////////////////
	{
		float heightC = hC.x + hC.y + hC.z;
		float heightR = hE.x + hE.y + hE.z;
		float heightL = hW.x + hW.y + hW.z;
		float heightU = hN.x + hN.y + hN.z;
		float heightD = hS.x + hS.y + hS.z;
		
		vec3 va = normalize(vec3(u_cellSize.x*2.0, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y*2.0));
		
		vec3 rockNormal = -cross(va,vb);
		out_normalData.zw = rockNormal.xz;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Water normal
	//////////////////////////////////////////////////////////////////////////////////
	{
		float heightC = hC.x + hC.y + hC.z + hC.w;
		float heightL = hW.x + hW.y + hW.z + hW.w;
		float heightR = hE.x + hE.y + hE.z + hE.w;
		float heightU = hN.x + hN.y + hN.z + hN.w;
		float heightD = hS.x + hS.y + hS.z + hS.w;

		vec3 va = normalize(vec3(u_cellSize.x*2.0, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y*2.0));

		vec3 waterNormal = -cross(va,vb);
		out_normalData.xy = waterNormal.xz;
	}
	
	//////////////////////////////////////////////////////////////////////////////////
	// Occlusion
	//////////////////////////////////////////////////////////////////////////////////
	{
		float occlusion = 0.0f;
		float heightC = hC.x + hC.y + hC.z;

		float strength = 1.0;
		float totalStrength = 0.0;
		for ( int i = 1; i < u_numHeightMips; i++ )
		{
			vec4 mippedHeightDataC = textureLod(s_heightData, in_uv, float(i));

			float mippedHeight = mippedHeightDataC.x + mippedHeightDataC.y + mippedHeightDataC.z;
			float diff = max(0.0f, mippedHeight - heightC);
			float ratio = diff / u_cellSize.x;
			float angle = atan(ratio);
			float occlusionFoThisMip = angle / HALF_PI;

			occlusion += occlusionFoThisMip * strength;
			totalStrength += strength;
			strength *= 1.25;
		}
		occlusion /= totalStrength;
		occlusion = min(1.0, occlusion);

		out_derivedData.x = occlusion;
	}
}