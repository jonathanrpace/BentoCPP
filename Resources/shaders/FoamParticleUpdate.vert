#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_velocity;
layout(location = 2) in vec4 in_properties;

// Uniforms
uniform sampler2D s_waterNormalData;
uniform sampler2D s_waterData;
uniform sampler2D s_rockData;
uniform sampler2D s_mappingData;
uniform float u_terrainSize = 1.5;

// Outputs
out Varying
{
	vec4 out_position;
	vec4 out_velocity;
};

void main(void)
{
	vec2 uv = vec2(in_position.x, in_position.z);

	vec4 waterNormal = texture2D( s_waterNormalData, uv );
	vec4 waterData = texture2D( s_waterData, uv );
	vec4 rockData = texture2D( s_rockData, uv );

	float solidHeight = rockData.x;
	float moltenHeight = rockData.y;
	float dirtHeight = rockData.w;
	float waterHeight = waterData.x;
	float waveHeight = waterData.w;

	float waterSurfaceHeight = solidHeight + moltenHeight + dirtHeight + waterHeight + waveHeight;

	float life = in_velocity.w;
	vec4 position = in_position;
	vec3 velocity = in_velocity.xyz;

	float spawnThreshold = mix( 0.2, 0.9, in_properties.w );
	vec4 mappingData = texture2D( s_mappingData, in_properties.xy );
	float foamSpawnStrength = mappingData.w;

	if ( life <= 0.0 )
	{
		if ( foamSpawnStrength > spawnThreshold )
		{
			life = pow( mix(0.7, 1.0, foamSpawnStrength), 0.8 );
			position.x = in_properties.x;
			position.z = in_properties.y;
		}
	}
	else
	{
		// Speed up death if needed back home
		if ( life > 0.0 && foamSpawnStrength > spawnThreshold )
		{
			life -= (1.0f/400.0);
		}
		else
		{
			life -= (1.0/800.0);
		}

		life = max(0,life);
	}

	velocity.y = 0;

	float damping = mix( 0.95, 0.9, in_properties.z );
	velocity *= damping;

	position.xyz += velocity;
	position.y = waterSurfaceHeight;

	float speed = mix(0.00006, 0.00014, in_properties.z);
	velocity.x += waterNormal.x * speed;
	velocity.z += waterNormal.z * speed;

	out_position = position;
	out_velocity = vec4(velocity, life);
} 