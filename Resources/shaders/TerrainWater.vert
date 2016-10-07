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
 
// Uniforms
uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_viewMatrix;

uniform float u_mapHeightOffset;
uniform float u_waterDepthToOpaque;
uniform float u_dissolvedDirtDepthToDiffuse;


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
	float out_alpha;
	float out_dirtAlpha;
	vec3 out_eyeVec;
	float out_specularOcclusion;
	vec2 out_waterVelocity;
	vec2 out_uv;
};

////////////////////////////////////////////////////////////////
// STD Lib Functions
////////////////////////////////////////////////////////////////
vec3 reconstructNormal( vec2 normal2 );

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
	vec4 normalDataC = textureLod(s_normalData, in_uv, 0);

	float solidHeight = heightDataC.x;
	float moltenHeight = heightDataC.y;
	float dirtHeight = heightDataC.z;
	float waterHeight = heightDataC.w;
	float dissolvedDirt = miscDataC.z;

	out_waterVelocity = velocityDataC.zw;
	out_uv = in_uv;

	vec3 normal = reconstructNormal(normalDataC.xy);
	out_normal = normal;

	vec4 position = vec4(in_position, 1.0f);
	position.y += solidHeight;
	position.y += moltenHeight;
	position.y += dirtHeight;
	position.y += waterHeight;
	position.y += miscDataC.y * u_mapHeightOffset;
	out_worldPosition = position;

	float alpha = min( waterHeight / u_waterDepthToOpaque, 1.0 );
	out_alpha = alpha;

	vec4 viewPosition = u_modelViewMatrix * position;
	viewPosition.w = 1.0;
	out_viewPosition = viewPosition;

	vec4 screenPos = u_mvpMatrix * position;
	//out_screenPosition = screenPos;
	gl_Position = screenPos;

	////////////////////////////////////////////////////////////////
	// Diffuse
	////////////////////////////////////////////////////////////////
	{
		//float lighting = diffuse(normal, lightDir, 1.5f) * u_lightIntensity;
		//lighting += u_ambientLightIntensity;
		float dissolvedDirtAlpha = min( dissolvedDirt / u_dissolvedDirtDepthToDiffuse, 1.0 );
		out_dirtAlpha = dissolvedDirtAlpha;

		//out_diffuse.a = dissolvedDirtAlpha;
		//out_diffuse.rgb = mix( pow( u_waterColor * 0.5, vec3(2.2) ), pow( u_dirtColor, vec3(2.2) ), dissolvedDirtAlpha );
		//out_diffuse.rgb *= lighting;
	}

	////////////////////////////////////////////////////////////////
	// Reflections
	////////////////////////////////////////////////////////////////
	{
		//out_reflections = vec3(0.0);

		out_eyeVec = -normalize( viewPosition.xyz * mat3(u_viewMatrix) );
		vec3 reflectVec = -reflect(out_eyeVec, normal);

		// Fresnel
		//float fresnel = 1.0f - clamp(dot(normal, eye), 0.0f, 1.0f);
		//fresnel = pow( fresnel, u_fresnelPower );
		//fresnel = mix( 0.5, 1.0, fresnel );

		// Specular
		//float waterSpecular = lightingGGX(normal, eye, lightDir, u_specularPower, 1.0) * u_lightIntensity * 0.1;
		//vec3 waterSpecular = vec3( specular( normal, lightDir, -eye, u_specularPower ) * u_lightIntensity );
		//out_reflections += waterSpecular * u_waterColor * fresnel;

		// Sky
		//float skyReflect = specular(normal, vec3(0.0,1.0,0.0), -eye, 1.0);
		//out_reflections += skyReflect * 0.5 * fresnel * u_ambientLightIntensity;

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