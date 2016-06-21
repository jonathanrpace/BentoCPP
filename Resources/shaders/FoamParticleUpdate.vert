#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_velocity;
layout(location = 2) in vec4 in_properties;

// Uniforms
uniform sampler2D s_heightData;
uniform sampler2D s_velocityData;
uniform float u_terrainSize = 1.5;
uniform float u_waterHeightToOpaque = 0.005;

// Outputs
out Varying
{
	vec4 out_position;
	vec4 out_velocity;
};

void main(void)
{
	vec2 uv = vec2(in_position.x, in_position.z);

	vec4 heightData = texture( s_heightData, uv );
	vec2 waterVelocity = texture( s_velocityData, uv ).zw;

	float solidHeight = heightData.x;
	float moltenHeight = heightData.y;
	float dirtHeight = heightData.z;
	float waterHeight = heightData.w;
	float waterSurfaceHeight = solidHeight + moltenHeight + dirtHeight + waterHeight;

	float life = in_velocity.w;
	vec4 position = in_position;
	vec3 velocity = in_velocity.xyz;

	//float spawnThreshold = mix( 0.0, 0.2, in_properties.w );
	float waterSpeed = length(waterVelocity);
	if (isnan(waterSpeed))
		waterSpeed = 0.0;
	if (isinf(waterSpeed))
		waterSpeed = 0.0;

	float foamSpawnStrength = waterSpeed;

	if ( life <= 0.0 )
	{
		//if ( foamSpawnStrength > spawnThreshold )
		//{
			life = 1.0;
			position.x = in_properties.x;
			position.z = in_properties.y;
		//}
	}
	else
	{
		float lifeFrames = mix( 50, 100, in_properties.z );
		life -= (1.0/lifeFrames);
		life = max(0,life);
	}

	position.xz += waterVelocity.xy * 0.0025;
	position.y = waterSurfaceHeight;

	float alpha = min( waterHeight / u_waterHeightToOpaque, 1.0 );
	position.w = alpha * max( waterSpeed, 0.05 );

	out_position = position;
	out_velocity = vec4(velocity, life);
} 