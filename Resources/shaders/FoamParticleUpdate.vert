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

	vec4 mappingData = texture2D( s_mappingData, in_properties.xy );
	float foamSpawnStrength = min( mappingData.w, 1.0 );

	float maxLife = mix(0.5, 1.0, in_properties.z);

	float solidHeight = rockData.x;
	float moltenHeight = rockData.y;
	float dirtHeight = rockData.w;
	float waterHeight = waterData.x;
	float iceHeight = waterData.y;

	float waterSurfaceHeight = solidHeight + moltenHeight + dirtHeight + waterHeight + iceHeight;

	float life = in_velocity.w;
	vec4 position = in_position;
	vec3 velocity = in_velocity.xyz;

	if ( life <= 0.0 && foamSpawnStrength > 0.1 )
	{
		life = maxLife * mix( 0.1, 1.0f, foamSpawnStrength);
		
		position.x = in_properties.x;
		position.z = in_properties.y;

		velocity = waterNormal.xyz * pow(foamSpawnStrength, 1.5) * 0.001;
	}

	const float GRAVITY = 0.0001;

	life -= (1.0/800.0);
	life = max(0,life);

	// Speed up death if needed back home
	if ( life > 0.0 && foamSpawnStrength > 0.1 )
	{
		life -= (1.0f/60.0);
		life = max(0,life);
	}
	
	bool wasInAir = position.y > waterSurfaceHeight;
	position.xyz += velocity;
	bool inAir = (position.y-GRAVITY*5.0) > waterSurfaceHeight;

	if ( wasInAir && !inAir )
	{
		velocity.y *= 0.1;
		velocity = reflect(velocity, waterNormal.xyz);
		velocity.y -= 0.001;
		velocity *= 0.995;
	}
	else if ( inAir )
	{
		// Gravity
		velocity.y -= GRAVITY;

		velocity *= 0.995;
	}
	else
	{
		velocity.y = 0;
		velocity *= 0.8;

		position.y = waterSurfaceHeight;

		float speed = 0.0003;
		velocity.x += waterNormal.x * speed;
		velocity.z += waterNormal.z * speed;
	}

	out_position = position;
	out_velocity = vec4(velocity, life);
} 