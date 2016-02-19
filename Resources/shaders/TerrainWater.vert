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
uniform float u_phase;

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

float waterNoiseHeight(vec2 uv, float waterHeight)
{
	const int NUM_OCTAVES = 4;
	const float OCTAVE_SCALE = 1.05;
	const float OCTAVE_STRENGTH = 0.5;
	const float OCTAVE_OFFSET = 1.0;
	const float OCTAVE_ANGLE = (3.142*2.0)/NUM_OCTAVES;

	float scale = 1.5;
	float strength = 1.0;
	float angle = 0.0;
	vec2 uvOffset = vec2(u_phase*1.0, u_phase * 1.0);

	uv *= scale;
	uv += uvOffset;

	float outValue = 0.0;
	for ( int i = 0; i < NUM_OCTAVES; i++ )
	{
		float sinTheta = sin(angle);
		float cosTheta = cos(angle);

		vec2 rotatedUV = vec2(uv.x * cosTheta - uv.y * sinTheta, uv.y * cosTheta + uv.x * sinTheta);

		outValue += (texture( s_diffuseMap, rotatedUV ).z-0.5) * strength * 2;

		scale *= OCTAVE_SCALE;
		strength *= OCTAVE_STRENGTH;
		uvOffset *= -OCTAVE_OFFSET;
		angle += OCTAVE_ANGLE;

		uv *= scale;
		uv += uvOffset;
	}

	float scalar = smoothstep( 0.0, 0.5, waterHeight ) * 0.25;

	return (outValue+0.5) * scalar * 0.5;
}

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
	
	vec4 position = vec4(in_position, 1.0f);
	position.y += solidHeight;
	position.y += moltenHeight;
	position.y += dirtHeight;
	position.y += iceHeight;
	position.y += waterHeight;

	vec2 p = vec2(in_uv.xy);
	float noiseC = waterNoiseHeight(p, waterDataC.y);
	position.y += noiseC;

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