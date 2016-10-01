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
	float life = in_velocity.w;
	vec4 position = in_position;
	vec3 velocity = in_velocity.xyz;

	// Alive - Kill it a  little
	if ( life > 0.0 && life <= 1.0 )
	{
		float lifeFrames = mix( 500, 600, in_properties.z );
		life -= (1.0/lifeFrames);
		life = max(0,life);

		// Just died. Put it in the delay queue
		if ( life == 0.0 )
		{
			life = 1.0 + in_properties.z * 2.0;
			position.x = in_properties.x;
			position.z = in_properties.y;
		}
		position.y += 0.00007;
	}
	else
	{
		vec2 uv = vec2(in_properties.x, in_properties.y);

		float spawnThreshold = 0.01;//mix( 0.01, 1.0, in_properties.w );
		vec4 smudgeData = texture( s_smudgeData, uv );
		float spawnStrength = min( smudgeData.z, 1.0 );
		
		if ( spawnStrength >= spawnThreshold )
		{
			life = max( life - 0.01, 1.0 );

			// Spawn it!
			if ( life <= 1.0 )
			{
				vec4 heightData = texture( s_heightData, uv );
				float solidHeight = heightData.x;
				float moltenHeight = heightData.y;
				float dirtHeight = heightData.z;
				float waterHeight = heightData.w;
				float surfaceHeight = solidHeight + moltenHeight + dirtHeight + waterHeight;

				position.y = surfaceHeight + 0.01;
				position.w = max(spawnStrength, 0.6);
			}
		}
	}

	out_position = position;
	out_velocity = vec4(velocity, life);
} 