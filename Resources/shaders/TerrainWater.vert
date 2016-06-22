#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

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
		out_diffuse.rgb *= pow(u_dirtColor*0.5, vec3(2.2));
		out_diffuse.a = min( dissolvedDirt / 0.005, 1.0 ) * alpha;
	}

	////////////////////////////////////////////////////////////////
	// Reflections
	////////////////////////////////////////////////////////////////
	{
		out_reflections = vec3(0.0);

		vec3 eye = -normalize( viewPosition.xyz * mat3(u_viewMatrix) );

		// Specular
		vec3 waterSpecular = vec3( specular( normal, u_lightDir, -eye, u_specularPower ) * u_lightIntensity );
		out_reflections += waterSpecular * 0.5;

		// Sky
		float skyReflect = clamp( (dot(reflect(-eye, normal), vec3(0.0,1.0,0.0)) + 1.0) * 0.5, 0.0, 1.0 );
		out_reflections += skyReflect * 0.5;

		// Fresnel
		float fresnel = 1.0f - clamp(dot(normal, eye), 0.0f, 1.0f);
		fresnel = pow( fresnel, u_fresnelPower );
		fresnel = mix( 0.02, 1.0, fresnel );

		out_reflections *= fresnel;
		out_reflections *= alpha;
	}
} 