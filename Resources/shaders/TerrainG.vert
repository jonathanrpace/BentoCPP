#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

// Textures
uniform sampler2D s_rockData;
uniform sampler2D s_rockFluxData;
uniform sampler2D s_rockNormalData;

uniform sampler2D s_waterData;
uniform sampler2D s_waterFluxData;
uniform sampler2D s_waterNormalData;

uniform sampler2D s_mappingData;
 
// Uniforms
uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_normalModelViewMatrix;
uniform float u_mapHeightOffset;

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
	vec3 out_viewNormal;
	vec4 out_viewPosition;
	vec4 out_rockData;
	vec4 out_waterData;
	vec4 out_rockNormal;
	vec4 out_waterNormal;
	vec4 out_waterFluxData;
};

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

void main(void)
{
	// Pluck some values out of the texture data
	vec4 rockDataC = texture(s_rockData, in_uv);
	vec4 waterDataC = texture(s_waterData, in_uv);
	vec4 mappingDataC = texture(s_mappingData, in_uv);
	vec4 rockNormalDataC = texture(s_rockNormalData, in_uv);
	vec4 waterNormalDataC = texture(s_waterNormalData, in_uv);
	vec4 waterFluxDataC = texture(s_waterFluxData, in_uv);

	float solidHeight = rockDataC.x;
	float moltenHeight = rockDataC.y;
	float dirtHeight = rockDataC.w;
	float waterHeight = waterDataC.x;
	float iceHeight = waterDataC.y;
	
	vec4 position = vec4(in_position, 1.0f);
	position.y += solidHeight;
	position.y += moltenHeight;
	position.y += dirtHeight;
	//position.y += iceHeight;
	//position.y += waterHeight;

	float heat = rockDataC.z;
	position.y += mappingDataC.y * u_mapHeightOffset * (1.0f - min(heat,1.0f));

	out_uv = in_uv;
	out_viewNormal = vec3(0.0f, 1.0f, 0.0f);
	out_viewPosition = u_modelViewMatrix * position;

	out_rockData = rockDataC;
	out_waterData = waterDataC;
	out_rockNormal = rockNormalDataC;
	out_waterNormal = waterNormalDataC;
	out_waterFluxData = waterFluxDataC;

	vec4 screenPos = u_mvpMatrix * position;

	gl_Position = screenPos;
} 