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
uniform float u_moltenAlphaScalar;
uniform float u_moltenAlphaPower;

uniform vec3 u_dirtColor;
uniform vec3 u_vegColor;
uniform float u_vegBump;

uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_normalModelViewMatrix;
uniform float u_mapHeightOffset;

uniform vec3 u_lightDir;
uniform float u_shadowPenumbra;

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
	vec4 out_viewPosition;
	vec4 out_forward;
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
	float vegAmount = smudgeDataC.w;
	float heat = miscDataC.x;
	float occlusion = 1.0f - miscDataC.w;
	//float moltenMapValue = clamp(miscDataC.y, 0.0, 1.0);
	vec3 viewDir = normalize(u_cameraPos);
	float steamStrength = smudgeDataC.z;
	float moltenMapValue = texture(s_moltenMapData, in_uv).x;

	vec4 position = vec4(in_position, 1.0f);
	position.y += rockHeight;
	position.y += moltenHeight;
	//position.y += dirtHeight;
	position.y += moltenMapValue * u_mapHeightOffset;
	//position.y += vegAmount * u_vegBump;

	vec4 viewPosition = u_modelViewMatrix * position;

	// Dirt
	/*
	vec3 dirtDiffuse = pow(u_dirtColor, vec3(2.2));// * mix(0.0, 1.0, diffuseData.z);
	dirtDiffuse *= pow( texture( s_dirtDiffuse, in_uv ).rgb, vec3(2.2) );

	float dirtAlpha = clamp((dirtHeight / 0.003), 0.0, 1.0);
	diffuse = mix(diffuse, dirtDiffuse, dirtAlpha);
	
	rockNormal += rockDetailBump * u_rockDetailBumpStrength * (1.0-dirtAlpha);
	rockNormal = normalize(rockNormal);

	// Vegetation
	vec3 vegDiffuse = pow(u_vegColor, vec3(2.2)) * mix(0.0, 1.0, diffuseData.z);
	float vegAlpha = clamp( vegAmount / 1.0, 0.0, 1.0 );
	diffuse = mix(diffuse, vegDiffuse, vegAlpha);
	roughness = mix( roughness, 0.7, vegAlpha );
	fresnel = mix( fresnel, 0.9, vegAlpha );
	*/

	// Molten
	float moltenAlpha = max( max(heat-0.2, 0.0) * u_moltenAlphaScalar, 0.0 );
	vec3 moltenColor = pow( mix( u_moltenColor, u_moltenColor * 1.25, moltenAlpha ), vec3(2.2) );
	
	// Bing it all together
	//vec3 outColor = (diffuse * (directLight + ambientlight));
	//outColor *= pow( clamp(1.0 - moltenAlpha, 0.0, 1.0), 4.0 );
	//outColor = mix( outColor, moltenColor, moltenAlpha );

	//outColor = mix( outColor, vec3(0.0,1.0,0.0), pow(steamStrength, 2.2) );


	// Shadowing
	float shadowing = 0.0;
	{
		const int maxSteps = 64;
		const float minStepLength = (1.0 / 256.0);

		float stepLength = (1.0 / maxSteps);
		float stepLengthScalar = 1.0;

		vec3 rayPos = position.xyz;
		vec3 rayDir = u_lightDir;

		vec3 rayDirRight = cross(rayDir, vec3(0.0,1.0,0.0));
		vec3 rayDirUp = cross(rayDir, rayDirRight);

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
					float distanceTravelled = length(rayPos - position.xyz);
					float penumbraSize = u_shadowPenumbra * distanceTravelled;
					vec3 rayPosL = rayPos - rayDirRight * penumbraSize;
					vec3 rayPosR = rayPos + rayDirRight * penumbraSize;
					vec3 rayPosU = rayPos + rayDirUp * penumbraSize;
					vec3 rayPosD = rayPos - rayDirUp * penumbraSize;

					vec4 heightSampleL = texture( s_heightData, rayPosL.xz + 0.5 );
					vec4 heightSampleR = texture( s_heightData, rayPosR.xz + 0.5 );
					vec4 heightSampleU = texture( s_heightData, rayPosU.xz + 0.5 );
					vec4 heightSampleD = texture( s_heightData, rayPosD.xz + 0.5 );

					float inShadowL = clamp( abs((heightSampleL.x + heightSampleL.y + heightSampleL.z) - rayPosL.y) / penumbraSize, 0.0, 1.0 );
					float inShadowR = clamp( abs((heightSampleR.x + heightSampleR.y + heightSampleR.z) - rayPosR.y) / penumbraSize, 0.0, 1.0 );
					float inShadowU = clamp( abs((heightSampleU.x + heightSampleU.y + heightSampleU.z) - rayPosU.y) / penumbraSize, 0.0, 1.0 );
					float inShadowD = clamp( abs((heightSampleD.x + heightSampleD.y + heightSampleD.z) - rayPosD.y) / penumbraSize, 0.0, 1.0 );

					shadowing = (inShadowL + inShadowR + inShadowU + inShadowD) * 0.25;

					//shadowing = 1.0;
					break;
				}

				stepLengthScalar = 0.5;	 // Start binary chop
				rayPos -= rayDir * stepLength;
			}

			stepLength *= stepLengthScalar;
		}
	}
	
	// Output
	{
		out_viewPosition = viewPosition;
		out_forward = vec4(0.0);//vec4(outColor, 1.0f);//vec4(directLight);//vec4(moltenMapValue);//vec4(outColor, 1.0f);
		out_uv = in_uv;
		out_dirtAlpha = 1.0;
		out_moltenColor = moltenColor;
		out_moltenAlpha = moltenAlpha;
		out_rockNormal = rockNormal;
		out_occlusion = occlusion;
		out_heat = heat;
		out_shadowing = shadowing;
		gl_Position = u_mvpMatrix * position;
	}
} 