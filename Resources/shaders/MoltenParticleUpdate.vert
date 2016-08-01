#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec2 in_direction;
layout(location = 2) in vec4 in_properties;

// Uniforms
uniform sampler2D s_heightData;
uniform sampler2D s_velocityData;

// Outputs
out Varying
{
	vec4 out_position;
	vec2 out_direction;
};

void main(void)
{
	vec2 uv = vec2(in_position.x, in_position.z);

	vec4 heightData = texture( s_heightData, uv );
	vec2 moltenVelocity = texture( s_velocityData, uv ).xy;

	float solidHeight = heightData.x;
	float moltenHeight = heightData.y;
	float surfaceHeight = solidHeight + moltenHeight;

	float life = in_position.w;
	vec3 position = in_position.xyz;

	if ( life <= 0.0 )
	{
		life = 1.0;
		position.x = in_properties.x;
		position.z = in_properties.y;
	}
	else
	{
		float speed = length(moltenVelocity);
		float lifeDecay = speed * mix( 0.1, 0.05, in_properties.w );
		life -= lifeDecay;
		life = max(0,life);
	}

	position.xz += moltenVelocity * 0.002;
	position.y = surfaceHeight;

	out_position = vec4(position, life);


	vec2 direction = in_direction;

	float dp = dot( normalize(moltenVelocity), normalize(direction) );
	dp = (dp + 1.0) * 0.5;
	dp = 1.0 - dp;

	direction += moltenVelocity * 0.1;// * dp;

	out_direction = direction;
} 