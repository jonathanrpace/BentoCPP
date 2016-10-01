#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_velocity;
layout(location = 2) in vec4 in_properties;

// Uniforms
uniform mat4 u_mvpMatrix;
uniform float u_terrainSize = 1.0;
uniform vec3 u_cameraPos;
uniform vec3 u_lightDir;

const float PI = 3.14159265359;
const float PI_2 = PI * 2.0;

uniform sampler2D s_miscData;

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
	float out_heat;
	float out_translucency;
};

void main(void)
{
	vec4 miscData = textureLod( s_miscData, in_position.xz, 3 );
	out_heat = miscData.x;

	vec4 position = in_position;
	position.xz *= u_terrainSize;
	position.xz -= u_terrainSize*0.5;
	position.w = 1.0;

	vec4 screenPos = u_mvpMatrix * position;

	gl_Position = screenPos;

	vec3 eyeVec = normalize( position.xyz - u_cameraPos );

	out_translucency = pow( max( dot(eyeVec, u_lightDir), 0.0 ), 2.0 );

	

	float life = in_velocity.w;
	float powedLife = pow( life, 1.8 );

	if ( life <= 0.0 || life > 1.0 )
		gl_PointSize = 0.0;
	else
		gl_PointSize = mix( 32, 150 + mix(0.0, 80, in_properties.z), 1.0-powedLife );

	out_color = vec4(1.0);
	out_life = life;
	out_angle = 0.0;//mix(0.0, PI_2, in_properties.z) + life * PI_2 * 0.01;
	out_offset = in_properties.z - life * mix( 0.1, 0.3, in_properties.w );

	float t = 1.0-life;

	float alpha = min(t / 0.2, 1.0) * (life);

	alpha = smoothstep(0.0, 1.0, alpha);

	out_alpha = sin( life * PI ) * in_position.w;
} 