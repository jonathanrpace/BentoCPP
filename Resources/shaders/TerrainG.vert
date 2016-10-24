#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

// Uniforms
uniform mat4 u_viewMatrix;

uniform vec3 u_fogColorAway;
uniform vec3 u_fogColorTowards;
uniform float u_fogHeight;
uniform float u_fogDensity;
uniform vec3 u_cameraPos;

uniform float u_moltenMapOffset;
uniform float u_rockDetailBumpStrength;
uniform float u_rockDetailDiffuseStrength;

uniform vec3 u_moltenColor;
uniform float u_moltenColorScalar;
uniform float u_moltenAlphaScalar;
uniform float u_moltenAlphaPower;

uniform float u_dirtHeightToOpaque;

uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_normalModelViewMatrix;
uniform float u_mapHeightOffset;

uniform vec3 u_lightDir;
uniform float u_lightDistance;

// Textures
uniform sampler2D s_heightData;
uniform sampler2D s_velocityData;
uniform sampler2D s_miscData;
uniform sampler2D s_normalData;
uniform sampler2D s_diffuseMap;
uniform sampler2D s_smudgeData;
uniform sampler2D s_moltenMapData;
uniform sampler2D s_dirtDiffuse;
uniform sampler2D s_rockDiffuse;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

// GL
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
	vec3 out_moltenColor;
	float out_moltenAlpha;
	vec3 out_rockNormal;
	float out_occlusion;
	float out_heat;
	float out_shadowing;
};

////////////////////////////////////////////////////////////////
// STD Lib Functions
////////////////////////////////////////////////////////////////
vec4 sampleCombinedMip( sampler2D _sampler, vec2 _uv, int _minMip, int _maxMip, float _downSampleScalar );
vec3 reconstructNormal( vec2 normal2 );
float lightingGGX( vec3 N, vec3 V, vec3 L, float roughness, float F0 );

////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////
void main(void)
{
	// Pluck some values out of the texture data
	vec4 heightDataC = texture(s_heightData, in_uv);
	vec4 velocityDataC = texture(s_velocityData, in_uv);
	vec4 miscDataC = textureLod(s_miscData, in_uv, 1);
	vec4 smudgeDataC = texture(s_smudgeData, in_uv);
	vec4 diffuseData = texture(s_diffuseMap, in_uv);
	
	vec2 rockNormalPacked = sampleCombinedMip(s_normalData, in_uv, 0, 1, 0.5).zw;
	rockNormalPacked /= vec2(1.0 + 0.5 + 0.25);
	vec3 rockNormal = normalize( reconstructNormal(rockNormalPacked) );

	// Common values
	float rockHeight = heightDataC.x;
	float moltenHeight = heightDataC.y;
	float dirtHeight = heightDataC.z;
	float waterHeight = heightDataC.w;
	float heat = miscDataC.x;
	float occlusion = 1.0f - miscDataC.w;
	vec3 viewDir = normalize(u_cameraPos);
	float steamStrength = smudgeDataC.z;
	float moltenMapValue = texture(s_moltenMapData, in_uv).x;

	vec4 position = vec4(in_position, 1.0f);
	position.y += rockHeight;
	position.y += moltenHeight;
	position.y += dirtHeight;
	position.y += moltenMapValue * u_mapHeightOffset;

	vec4 viewPosition = u_modelViewMatrix * position;

	// Dirt
	float dirtAlpha = min((dirtHeight / u_dirtHeightToOpaque), 1.0);
	
	// Molten
	float moltenAlpha = min( max(heat-0.2, 0.0) * u_moltenAlphaScalar, 1.0 );
	moltenAlpha = pow( moltenAlpha, u_moltenAlphaPower );
	vec3 moltenColor = pow( mix( u_moltenColor, u_moltenColor * u_moltenColorScalar, moltenAlpha ), vec3(2.2) );
	
	// Shadowing
	float shadowing = 0.0;
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
	
	// Output
	{
		out_worldPosition = position.xyz;
		out_viewPosition = viewPosition;
		out_uv = in_uv;
		out_moltenColor = moltenColor;
		out_moltenAlpha = moltenAlpha;
		out_dirtAlpha = dirtAlpha;
		out_rockNormal = rockNormal;
		out_occlusion = occlusion;
		out_heat = heat;
		out_shadowing = shadowing;
		gl_Position = u_mvpMatrix * position;
	}
} 