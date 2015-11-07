#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

// Textures
uniform sampler2D s_heightData;
uniform sampler2D s_fluxData;
uniform sampler2D s_velocityData;
uniform sampler2D s_mappingData;

uniform sampler2D s_diffuseMap;
 
// Uniforms
uniform mat4 u_mvpMatrix;
uniform mat4 u_modelMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_normalModelViewMatrix;

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
	vec3 out_viewNormal;
	vec4 out_viewPosition;
	vec4 out_data0;
	vec4 out_data1;
	vec4 out_data2;
	vec4 out_diffuse;
};

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

void main(void)
{
	// Pluck some values out of the texture data
	vec4 heightDataSample = texture(s_heightData, in_uv);
	vec4 fluxDataSample = texture(s_fluxData, in_uv);
	vec4 velocityDataSample = texture(s_velocityData, in_uv);
	vec4 mappingDataSample = texture(s_mappingData, in_uv);

	float solidHeight = heightDataSample.x;
	float moltenHeight = heightDataSample.y;

	vec4 position = vec4(in_position, 1.0f);
	position.y += solidHeight;
	position.y += moltenHeight;

	vec2 uv = in_uv + mappingDataSample.xy;
	vec4 diffuseSample = texture(s_diffuseMap, uv);
	out_diffuse = diffuseSample;

	out_viewNormal = vec3(0.0f, 1.0f, 0.0f);
	out_viewPosition = u_modelViewMatrix * position;

	out_data0 = heightDataSample;
	out_data1 = fluxDataSample;
	out_data2 = velocityDataSample;

	gl_Position = u_mvpMatrix * position;
} 