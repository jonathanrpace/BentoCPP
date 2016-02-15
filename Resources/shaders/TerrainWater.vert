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
 
// Uniforms
uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;
uniform vec4 u_phase;

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
	vec2 out_foamUVA;
	vec2 out_foamUVB;
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
	float waterHeight = waterDataC.x;
	float iceHeight = waterDataC.y;
	
	vec4 position = vec4(in_position, 1.0f);
	position.y += solidHeight;
	position.y += moltenHeight;
	position.y += dirtHeight;
	position.y += iceHeight;
	position.y += waterHeight;

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


	vec4 waterFluxDataMipped = textureLod(s_waterFluxData, in_uv, 4);
	vec2 foamFlow = -vec2( waterFluxDataMipped.y - waterFluxDataMipped.x, waterFluxDataMipped.w - waterFluxDataMipped.z );

	float foamFlowSpeedA = length(foamFlow) * 0.6;
	float foamFlowSpeedB = length(foamFlow) * 0.6;
	foamFlowSpeedA = pow(foamFlowSpeedA, 0.8) * u_phase.z;
	foamFlowSpeedB = pow(foamFlowSpeedB, 0.8) * u_phase.w;

	vec2 foamFlowDirA = normalize(foamFlow) * foamFlowSpeedA;
	vec2 foamFlowDirB = normalize(foamFlow) * foamFlowSpeedB;

	vec2 foamUVA = in_uv + foamFlowDirA;
	vec2 foamUVB = in_uv + foamFlowDirB + vec2(0.5);

	out_foamUVA = foamUVA;
	out_foamUVB = foamUVB;

	gl_Position = screenPos;
} 