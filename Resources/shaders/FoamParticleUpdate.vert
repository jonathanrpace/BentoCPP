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

	float maxLife = 1.0f;//mix(0.9, 1.0, in_properties.z);

	float solidHeight = rockData.x;
	float moltenHeight = rockData.y;
	float dirtHeight = rockData.w;
	float waterHeight = waterData.x;
	float iceHeight = waterData.y;

	float life = in_velocity.w;
	vec4 position = in_position;
	vec3 velocity = in_velocity.xyz;

	if ( life <= 0.0 && foamSpawnStrength > 0.2 )
	{
		life = maxLife * mix( 0.1, 1.0f, foamSpawnStrength);
		
		position.x = in_properties.x;
		position.z = in_properties.y;

		velocity = vec3(waterNormal.x, 0.0, waterNormal.z) * 0.004;
	}

	life -= (1.0/800.0);
	life = max(0,life);

	// Speed up death if needed back home
	if ( life > 0.0 && foamSpawnStrength > 0.2 )
	{
		life -= (1.0f/60.0);
		life = max(0,life);
	}
	
	
	velocity *= 0.8;

	float speed = 0.0004;
	velocity.x += waterNormal.x * speed;
	velocity.z += waterNormal.z * speed;
	
	
	position.xz += velocity.xz;

	position.y = 0;
	position.y += solidHeight;
	position.y += moltenHeight;
	position.y += dirtHeight;
	position.y += iceHeight;
	position.y += waterHeight;

	out_position = position;
	out_velocity = vec4(velocity, life/maxLife);
} 