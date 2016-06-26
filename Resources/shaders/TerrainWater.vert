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
uniform float u_waterHeightToOpaque = 0.005;
uniform float u_fresnelPower = 4.0f;
uniform float u_specularPower;
uniform vec3 u_dirtColor;
const float u_terrainSize = 1.5;

// Lighting
uniform vec3 u_lightDir;
uniform float u_lightIntensity;
uniform float u_ambientLightIntensity;


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
	vec4 out_diffuse;
	vec3 out_reflections;
	float out_alpha;
};

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

vec3 reconstructNormal( vec2 normal2 )
{
	float len = length(normal2);
	vec3 normal3 = vec3(normal2.x, 1.0-len, normal2.y);
	return normalize(normal3);
}

float diffuse(vec3 n, vec3 l, float p)
{
    return pow(dot(n,l) * 0.5 + 1.0, p);
}

float specular(vec3 n, vec3 l, vec3 e, float s)
{    
    float nrm = (s + 8.0) / (3.1415 * 8.0);
    return pow(max(dot(reflect(e,n),l),0.0),s) * nrm;
}

void main(void)
{
	// Pluck some values out of the texture data
	vec4 heightDataC = texture(s_heightData, in_uv);
	vec4 velocityDataC = texture(s_velocityData, in_uv);
	vec4 miscDataC = texture(s_miscData, in_uv);
	vec4 normalDataC = texture(s_normalData, in_uv);

	float solidHeight = heightDataC.x;
	float moltenHeight = heightDataC.y;
	float dirtHeight = heightDataC.z;
	float waterHeight = heightDataC.w;
	float dissolvedDirt = miscDataC.z;

	vec3 normal = reconstructNormal(normalDataC.xy);
	out_normal = normal;

	vec4 position = vec4(in_position, 1.0f);
	position.y += solidHeight;
	position.y += moltenHeight;
	position.y += dirtHeight;
	position.y += waterHeight;
	position.y += miscDataC.y * u_mapHeightOffset;
	out_worldPosition = position;

	float alpha = min( waterHeight / u_waterHeightToOpaque, 1.0 );
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
		out_diffuse = vec4( vec3(diffuse(normal, u_lightDir, 1.5f) * u_lightIntensity), 0.0 );
		out_diffuse.rgb += u_ambientLightIntensity;

		// Water color is dissolved dirt color
		out_diffuse.rgb *= pow(u_dirtColor*0.25, vec3(2.2));
		out_diffuse.a = min( dissolvedDirt / 0.01, 1.0 ) * alpha;
	}

	////////////////////////////////////////////////////////////////
	// Reflections
	////////////////////////////////////////////////////////////////
	{
		out_reflections = vec3(0.0);

		vec3 eye = -normalize( viewPosition.xyz * mat3(u_viewMatrix) );
		vec3 reflectVec = reflect(-eye, normal);

		// Specular
		vec3 waterSpecular = vec3( specular( normal, u_lightDir, -eye, u_specularPower ) * u_lightIntensity );
		out_reflections += waterSpecular * 0.5;

		// Sky
		float skyReflect = clamp( (dot(reflectVec, vec3(0.0,1.0,0.0)) + 1.0) * 0.5, 0.0, 1.0 );
		out_reflections += skyReflect * 0.5;

		// Fresnel
		float fresnel = 1.0f - clamp(dot(normal, eye), 0.0f, 1.0f);
		fresnel = pow( fresnel, u_fresnelPower );
		fresnel = mix( 0.02, 1.0, fresnel );

		// Specular occlusion
		const int maxSteps = 16;
		const float minStepLength = (1.0 / 1024.0);

		vec3 occlusionPos = position.xyz;
		float stepLength = (u_terrainSize / maxSteps) * 0.5;	// Walk at most, a quarter the size of the terrain
		float occlusion = 0.0;
		bool binaryChopping = false;

		// Jitter
		vec3 randomDirection = normalize( vec3( in_rand.xyz - 0.5 ) );
		reflectVec += randomDirection * 0.03;

		for ( int i = 0; i < maxSteps; i++ )
		{
			occlusionPos += reflectVec * stepLength;

			vec2 sampleUV = (occlusionPos.xz + (u_terrainSize*0.5)) / u_terrainSize;

			if ( sampleUV.x < 0.0 )	break;
			if ( sampleUV.x > 1.0 )	break;
			if ( sampleUV.y < 0.0 )	break;
			if ( sampleUV.y > 1.0 )	break;

			vec4 heightSample = textureLod(s_heightData, sampleUV, 1);
			float terrainHeightAtSample = heightSample.x + heightSample.y + heightSample.z;

			if ( terrainHeightAtSample > occlusionPos.y )
			{
				if ( i == (maxSteps-1) || stepLength <= minStepLength )
				{
					occlusion = 1.0;
					break;
				}

				binaryChopping = true;
				occlusionPos -= reflectVec * stepLength;
			}

			if ( binaryChopping )
			{
				stepLength *= 0.5;
			}
		}

		out_reflections *= fresnel;
		out_reflections *= alpha;
		out_reflections *= 1.0-occlusion;
	}
} 