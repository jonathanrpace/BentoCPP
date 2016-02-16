#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec3 in_velocity;
layout(location = 2) in vec4 in_properties;

// Uniforms
uniform mat4 u_mvpMatrix;
uniform float u_terrainSize = 1.5;

uniform sampler2D s_waterData; 
uniform sampler2D s_rockData; 

// Outputs
out gl_PerVertex 
{
	vec4 gl_Position;
	float gl_PointSize;
};

void main(void)
{
	

	vec2 uv = (vec2(in_position.x, in_position.z) / u_terrainSize) + 0.5;
	vec4 waterData = texture2D( s_waterData, uv );
	vec4 rockData = texture2D( s_rockData, uv );

	vec4 position = in_position;
	//position.y += waterData.y + 0.001f;

	vec4 screenPos = u_mvpMatrix * position;
	//gl_PointSize = (1.0-screenPos.z / screenPos.w) * 2000.0f;
	gl_PointSize = 1.0f / (1.0f+screenPos.z);
	gl_Position = screenPos;
} 