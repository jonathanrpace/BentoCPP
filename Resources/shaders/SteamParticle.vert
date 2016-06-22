#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_velocity;
layout(location = 2) in vec4 in_properties;

// Uniforms
uniform mat4 u_mvpMatrix;
uniform float u_terrainSize = 1.5;

// Outputs
out gl_PerVertex 
{
	vec4 gl_Position;
	float gl_PointSize;
};

out Varying
{
	vec4 out_color;
	float out_angle;
	float out_life;
	float out_alpha;
	float out_offset;
};

void main(void)
{
	vec4 position = in_position;
	position.xz *= u_terrainSize;
	position.xz -= u_terrainSize*0.5;
	position.w = 1.0;

	vec4 screenPos = u_mvpMatrix * position;

	gl_Position = screenPos;

	float life = in_velocity.w;
	float lifeAlpha = pow( min( (1.0-life)/0.2, 1.0 ) * life, 0.75 );
	
	lifeAlpha = min(lifeAlpha, 1.0);
	//float lifeAlpha = sin(life*3.142);
	
	if ( life <= 0.0 )
		gl_PointSize = 0.0;
	else
		gl_PointSize = 16 + 16 * mix( 2.0, 8.0, in_properties.z );

	out_color = vec4(vec3(1.0), lifeAlpha);

	out_life = life;

	const float PI_2 = 3.142 * 2.0;
	out_angle = mix(0.0, PI_2, in_properties.z) + life * PI_2 * mix( 0.1, 0.3, in_properties.w );

	out_offset = life * mix( 0.6, 1.2, in_properties.w );

	out_alpha = in_position.w;
} 