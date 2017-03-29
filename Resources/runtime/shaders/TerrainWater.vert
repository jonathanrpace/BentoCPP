#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_rand;

// Textures
uniform sampler2D s_heightData;
uniform sampler2D s_velocityData;
uniform sampler2D s_miscData;
uniform sampler2D s_normalData;
uniform sampler2D s_smudgeData;
uniform sampler2D s_fluxData;
 
// Uniforms
uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_viewMatrix;

uniform float u_heightOffset;
uniform float u_depthToReflect;
uniform float u_dissolvedDirtDensityScalar;


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
	vec3 out_normal;
	vec4 out_viewPosition;
	vec4 out_worldPosition;
	float out_reflectAlpha;
	float out_dissolvedDirtAlpha;
	vec3 out_eyeVec;
	float out_specularOcclusion;
	vec2 out_waterVelocity;
	vec2 out_uv;
	float out_foamStrength;
	float out_fluxAmount;
};

////////////////////////////////////////////////////////////////
// STD Lib Functions
////////////////////////////////////////////////////////////////
vec3 reconstructNormal( vec2 n )
{
	float len = length(n);
	return normalize( vec3(n.x, (1.0-len), n.y) );
}

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

float diffuse(vec3 n, vec3 l, float p)
{
    return pow(dot(n,l) * 0.5 + 1.0, p);
}

float specular(vec3 n, vec3 l, vec3 e, float s)
{    
    float nrm = (s + 8.0) / (3.1415 * 8.0);
    return pow(max(dot(reflect(e,n),l),0.0),s) * nrm;
}

////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////
void main(void)
{
	// Pluck some values out of the texture data
	vec4 heightDataC = texture(s_heightData, in_uv);
	vec4 velocityDataC = texture(s_velocityData, in_uv);
	vec4 miscDataC = texture(s_miscData, in_uv);
	vec4 normalDataC = texture(s_normalData, in_uv);
	vec4 smudgeDataC = texture(s_smudgeData, in_uv);
	vec4 fluxDataC = texture(s_fluxData, in_uv);

	vec4 absFlux = abs(fluxDataC);
	out_fluxAmount = (absFlux.x + absFlux.y + absFlux.z + absFlux.w) * 100.0;

	float solidHeight = heightDataC.x;
	float moltenHeight = heightDataC.y;
	float dirtHeight = heightDataC.z;
	float waterHeight = heightDataC.w;
	float dissolvedDirt = miscDataC.z;

	out_foamStrength = min( smudgeDataC.w, 1.0 );

	out_waterVelocity = velocityDataC.zw;
	out_uv = in_uv;

	vec3 normal = reconstructNormal(normalDataC.xy);
	out_normal = normal;

	vec4 position = vec4(in_position, 1.0f);
	position.y += solidHeight;
	position.y += moltenHeight;
	position.y += dirtHeight;
	position.y += waterHeight;
	out_worldPosition = position;

	float reflectAlpha = min( max(waterHeight, 0.0) / u_depthToReflect, 1.0 );
	out_reflectAlpha = reflectAlpha;

	vec4 viewPosition = u_modelViewMatrix * position;
	viewPosition.w = 1.0;
	out_viewPosition = viewPosition;

	vec4 screenPos = u_mvpMatrix * position;
	gl_Position = screenPos;

	////////////////////////////////////////////////////////////////
	// Dissolved Dirt Alpha
	////////////////////////////////////////////////////////////////
	{
		float dissolvedDirtAlpha = min( dissolvedDirt * u_dissolvedDirtDensityScalar, 1.0 );
		out_dissolvedDirtAlpha = dissolvedDirtAlpha;
	}

	////////////////////////////////////////////////////////////////
	// Specular occlusion
	////////////////////////////////////////////////////////////////
	{
		out_eyeVec = -normalize( viewPosition.xyz * mat3(u_viewMatrix) );
		vec3 reflectVec = -reflect(out_eyeVec, normal);

		// Specular occlusion
		float shadowing = 0.0;
		{
			const int maxSteps = 64;
			const float minStepLength = (1.0 / 256.0);

			float stepLength = (1.0 / maxSteps);
			float stepLengthScalar = 1.0;

			vec3 rayPos = position.xyz;
			vec3 rayDir = reflectVec;

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
		
		out_specularOcclusion = 1.0-shadowing;
	}
} 