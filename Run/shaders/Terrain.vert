#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

// Uniforms
uniform mat4 u_viewMatrix;

uniform float u_uvRepeat;
uniform float u_displacement;

uniform vec3 u_fogColorAway;
uniform vec3 u_fogColorTowards;
uniform float u_fogHeight;
uniform float u_fogDensity;
uniform vec3 u_cameraPos;

uniform float u_moltenMapOffset;

uniform vec3 u_moltenColor;
uniform float u_moltenColorScalar;
uniform float u_moltenAlphaScalar;
uniform float u_moltenAlphaPower;


uniform float u_smudgeUVStrength;
uniform float u_smudgeSampleOffset;
uniform float u_smudgeSampleMip;

uniform float u_dirtHeightToOpaque;

uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_normalModelViewMatrix;
uniform float u_heightOffset;

uniform vec3 u_lightDir;
uniform float u_lightDistance;

// Textures
uniform sampler2D s_heightData;
uniform sampler2D s_miscData;
uniform sampler2D s_normalData;
uniform sampler2D s_smudgeData;
uniform sampler2D s_lavaMaterial;
uniform samplerCube s_envMap;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

out gl_PerVertex
{
	vec4 gl_Position;
};

// Varying
out Varying
{
	vec3 out_worldPosition;
	vec4 out_viewPosition;
	vec2 out_uv;
	float out_dirtAlpha;
	float out_heat;
	vec3 out_rockNormal;
	float out_occlusion;
	float out_shadowing;
	vec2 out_scaledUV;
	vec4 out_heightData;
	vec4 out_smudgeData;
	vec3 out_albedoFluidColor;
	vec4 out_miscData;
};

////////////////////////////////////////////////////////////////
// STD Lib Functions
////////////////////////////////////////////////////////////////
vec4 sampleCombinedMip( sampler2D _sampler, vec2 _uv, int _minMip, int _maxMip, float _downSampleScalar );

float packUnorm4x8f( vec4 );
vec4 unpackUnorm4x8f( float );

vec3 reconstructNormal( vec2 n )
{
	float len = length(n);
	return normalize( vec3(n.x, (1.0-len), n.y) );
}

////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////
void main(void)
{
	// Pluck some values out of the texture data
	vec4 heightDataC = texture(s_heightData, in_uv);
	vec4 miscDataC = textureLod(s_miscData, in_uv, 0);
	vec4 smudgeDataC = texture(s_smudgeData, in_uv);
	
	out_albedoFluidColor = unpackUnorm4x8f( smudgeDataC.z ).rgb;
	
	vec2 rockNormalPacked = sampleCombinedMip(s_normalData, in_uv, 0, 1, 0.5).zw;
	rockNormalPacked /= vec2(1.0 + 0.5 + 0.25);
	vec3 rockNormal = normalize( reconstructNormal(rockNormalPacked) );

	// Common values
	float rockHeight = heightDataC.x;
	float moltenHeight = heightDataC.y;
	float dirtHeight = heightDataC.z;
	float waterHeight = heightDataC.w;
	float heat = miscDataC.x;
	float occlusion = max(1.0f - miscDataC.w * 1.4f, 0.0);
	vec3 viewDir = normalize(u_cameraPos);
	float steamStrength = smudgeDataC.z;

	{
		vec2 outScaledUV = vec2(0.0);
		out_smudgeData = texture(s_smudgeData, in_uv);
		vec2 smudgeVec = textureLod( s_smudgeData, in_uv, u_smudgeSampleMip ).xy;
		vec2 smudgeDir = normalize(smudgeVec);

		vec2 uvOffset = smudgeVec * u_smudgeUVStrength;
		outScaledUV -= uvOffset;

		outScaledUV += in_uv;
		out_scaledUV = outScaledUV * u_uvRepeat;
	}

	vec4 materialSample = texture(s_lavaMaterial, out_scaledUV);
	float materialHeight = materialSample.a;


	vec4 position = vec4(in_position, 1.0f);
	position.y += rockHeight;
	position.y += moltenHeight;
	position.y += dirtHeight;

	position.xyz += rockNormal * (materialHeight) * u_displacement;

	vec4 viewPosition = u_modelViewMatrix * position;

	// Dirt
	float dirtAlpha = min((dirtHeight / u_dirtHeightToOpaque), 1.0);
	
	// Shadowing
	float shadowing = 0.0;
	/*
	{
		const int maxSteps = 64;
		const float minStepLength = (1.0 / 256.0);

		float stepLength = (1.0 / maxSteps);
		float stepLengthScalar = 1.0;

		vec3 lightPos = u_lightDir * u_lightDistance;
		vec3 rayPos = position.xyz;
		vec3 rayDir = normalize(lightPos - position.xyz);

		for ( int i = 0; i < maxSteps; i++ )
		{
			rayPos += rayDir * stepLength;

			vec2 sampleUV = rayPos.xz + 0.5;

			vec4 heightSample = texture(s_heightData, sampleUV);
			float terrainHeightAtSample = heightSample.x + heightSample.y + heightSample.z;

			if ( terrainHeightAtSample > rayPos.y )
			{
				if ( i == (maxSteps-1) || stepLength <= minStepLength )
				{
					shadowing = 1.0;
					break;
				}

				stepLengthScalar = 0.5;
				rayPos -= rayDir * stepLength;
			}

			stepLength *= stepLengthScalar;
		}
	}
	*/
	
	// Output
	{
		out_worldPosition = position.xyz;
		out_viewPosition = viewPosition;
		out_uv = in_uv;
		out_heat = heat;
		out_dirtAlpha = dirtAlpha;
		out_rockNormal = rockNormal;
		out_occlusion = occlusion;
		out_shadowing = shadowing;
		gl_Position = u_mvpMatrix * position;
		out_heightData = heightDataC;
		out_miscData = miscDataC;
	}
} 