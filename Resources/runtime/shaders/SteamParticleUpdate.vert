#version 330 core

////////////////////////////////
// Inputs
////////////////////////////////

	// Attributes
	layout(location = 0) in vec4 in_data0;
	layout(location = 1) in vec4 in_data1;
	layout(location = 2) in vec4 in_data2;
	layout(location = 3) in vec4 in_properties;

	// Textures
	uniform sampler2D s_heightData;
	uniform sampler2D s_miscData;

	// Uniforms
	uniform float u_dt;

	uniform vec2 u_spawnThreshold;
	uniform vec2 u_spawnDelay;
	uniform vec2 u_life;
	uniform vec3 u_spawnVelocityMin;
	uniform vec3 u_spawnVelocityMax;
	uniform float u_spawnSize;

	uniform vec3 u_positionAccelerationMin;
	uniform vec3 u_positionAccelerationMax;
	uniform vec2 u_positionDamping;

	uniform vec2 u_sizeAcceleration;
	uniform vec2 u_sizeDamping;

////////////////////////////////
// Outputs
////////////////////////////////

	out Varying
	{
		vec4 out_data0;
		vec4 out_data1;
		vec4 out_data2;
	};

////////////////////////////////
// Functions
////////////////////////////////
void srand(float _seed);
float rand();

vec3 rangedParam( vec3 _min, vec3 _max )
{
	return mix( _min, _max, rand() );
}

float rangedParam( vec2 _minMax )
{
	return mix( _minMax.x, _minMax.y, rand() );
}

float rangedParam( float _min, float _max )
{
	return mix( _min, _max, rand() );
}

////////////////////////////////
// Main
////////////////////////////////
void main(void)
{
	srand(in_properties.x);

	// Spawn/Life params
	float maxLife = rangedParam(u_life);
	float spawnThreshold = rangedParam(u_spawnThreshold);
	float spawnDelay = rangedParam(u_spawnDelay);
	vec3 spawnVelocity = rangedParam(u_spawnVelocityMin, u_spawnVelocityMax);

	// Motion params
	vec3 positionAcceleration = rangedParam(u_positionAccelerationMin, u_positionAccelerationMax);
	float positionDamping = rangedParam(u_positionDamping);
	float sizeAcceleration = rangedParam(u_sizeAcceleration);
	float sizeDamping = rangedParam(u_sizeDamping);

	float currLife = in_data2.x;
	vec3 position = in_data0.xyz;
	float size = in_data0.w;
	vec3 velocity = in_data1.xyz;
	float sizeVelocity = in_data1.w;
	float lifeNrm = -1.0;

	bool alive = false;

	// Alive
	if ( currLife > 0.0 && currLife <= maxLife )
	{
		// Reduce life
		currLife -= u_dt;
		lifeNrm = clamp( 1.0 - (currLife / maxLife), 0.0, 1.0 );

		alive = true;

		// Just died. Set its life to something above its max life.
		// It will be spawned when it reaches maxLife
		if ( currLife <= 0.0f )
		{
			currLife = maxLife + spawnDelay;
			alive = false;
		}

		// Update motion
		velocity += positionAcceleration;
		position += velocity;
		velocity -= velocity * positionDamping;

		// Update size motion
		sizeVelocity += sizeAcceleration;
		size += sizeVelocity;
		sizeVelocity -= sizeVelocity * sizeDamping;

		
	}
	// Waiting to spawn
	else
	{
		vec2 uv = vec2(in_properties.x, in_properties.y);

		float heat = texture( s_miscData, uv ).x;
		float spawnChance = heat;
		
		// Potentially spawnable
		if ( spawnChance >= spawnThreshold )
		{
			// Reduce life
			currLife -= u_dt;

			// SPAWN (If delay finised)
			if ( currLife <= maxLife )
			{
				currLife = maxLife;

				vec4 heightData = texture( s_heightData, uv );
				float solidHeight = heightData.x;
				float moltenHeight = heightData.y;
				float dirtHeight = heightData.z;
				float waterHeight = heightData.w;
				float surfaceHeight = solidHeight + moltenHeight + dirtHeight + waterHeight;

				// Reset position
				position.x = in_properties.x;
				position.z = in_properties.y;
				position.y = surfaceHeight;

				velocity = spawnVelocity;

				size = u_spawnSize;
				sizeVelocity = 0.0;

				alive = true;
			}
		}
		else
		{
			currLife = maxLife + spawnDelay;
		}
	}
	
	out_data0 = vec4(position, size);
	out_data1 = vec4(velocity, sizeVelocity);
	out_data2 = vec4(currLife, lifeNrm, alive ? 1.0 : 0.0, 0.0);
} 