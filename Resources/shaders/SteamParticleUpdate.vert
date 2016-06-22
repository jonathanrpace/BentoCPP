#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_velocity;
layout(location = 2) in vec4 in_properties;

// Uniforms
uniform sampler2D s_heightData;
uniform sampler2D s_smudgeData;

// Outputs
out Varying
{
	vec4 out_position;
	vec4 out_velocity;
};

void main(void)
{
	vec2 uv = vec2(in_properties.x, in_properties.y);

	vec4 heightData = texture( s_heightData, uv );
	vec4 smudgeData = texture( s_smudgeData, uv );

	float spawnStrength = min( smudgeData.z, 1.0 );
	float solidHeight = heightData.x;
	float moltenHeight = heightData.y;
	float dirtHeight = heightData.z;
	float waterHeight = heightData.w;
	float surfaceHeight = solidHeight + moltenHeight + dirtHeight + waterHeight;
	float spawnThreshold = in_properties.w;//mix( 0.01, 1.0, in_properties.w );

	float life = in_velocity.w;
	vec4 position = in_position;
	vec3 velocity = in_velocity.xyz;

	if ( life <= 0.0 )
	{
		if ( abs( spawnStrength - spawnThreshold ) < 0.01 )
		{
			life = 1.0;
			position.x = in_properties.x;
			position.z = in_properties.y;
			position.y = surfaceHeight;
			velocity = vec3( 0.0, mix(0.0002, 0.0003, in_properties.z), 0.0 );
			position.w = spawnStrength;
		}
	}
	else
	{
		float lifeFrames = mix( 600, 1000, in_properties.z );
		life -= (1.0/lifeFrames);
		life = max(0,life);
	}

	position.xyz += velocity;

	velocity += vec3( 0.0000015, 0.0, 0.0000015 );
	velocity *= 0.98;

	out_position = position;
	out_velocity = vec4(velocity, life);
} 