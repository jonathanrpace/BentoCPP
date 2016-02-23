#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

// Textures
uniform sampler2D s_rockData;
uniform sampler2D s_waterData;
uniform sampler2D s_waterFluxData;
uniform sampler2D s_waterNormalData;
uniform sampler2D s_rockNormalData;
uniform sampler2D s_diffuseMap;
 
// Uniforms
uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;


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
	vec2 out_uv;
	vec4 out_rockData;
	vec4 out_waterData;
	vec4 out_waterNormal;
	vec4 out_rockNormal;
	vec4 out_waterFluxData;
	vec4 out_viewPosition;
	vec4 out_worldPosition;
	vec4 out_screenPosition;
};

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

void main(void)
{
	// Pluck some values out of the texture data
	vec4 rockDataC = texture(s_rockData, in_uv);
	vec4 waterDataC = texture(s_waterData, in_uv);
	vec4 waterNormalDataC = texture(s_waterNormalData, in_uv);
	vec4 rockNormalDataC = texture(s_rockNormalData, in_uv);
	vec4 waterFluxDataC = texture(s_waterFluxData, in_uv);

	float solidHeight = rockDataC.x;
	float moltenHeight = rockDataC.y;
	float dirtHeight = rockDataC.w;
	float waterHeight = waterDataC.y;
	float iceHeight = waterDataC.x;
	float waveNoiseHeight = waterDataC.w;
	
	vec4 position = vec4(in_position, 1.0f);
	position.y += solidHeight;
	position.y += moltenHeight;
	position.y += dirtHeight;
	position.y += iceHeight;
	position.y += waterHeight;
	position.y += waveNoiseHeight;

	out_uv = in_uv;
	out_rockData = rockDataC;
	out_waterData = waterDataC;
	out_waterNormal = waterNormalDataC;
	out_rockNormal = rockNormalDataC;
	out_waterFluxData = waterFluxDataC;
	vec4 viewPosition = u_modelViewMatrix * position;
	viewPosition.w = 1.0;
	out_viewPosition = viewPosition;

	out_worldPosition = position;

	vec4 screenPos = u_mvpMatrix * position;

	out_screenPosition = screenPos;

	gl_Position = screenPos;
} 