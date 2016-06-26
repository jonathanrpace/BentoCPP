#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_properties;

uniform sampler2D s_velocityData;

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
};

void main(void)
{
	vec4 velocityData = texture(s_velocityData, in_position.xz);

	float speed = length(velocityData.xy) * 20.0;

	vec4 screenPos = vec4(in_position.x, in_position.z, 1.0, 1.0);
	screenPos.xy -= 0.5;
	screenPos.xy *= 2.0;
	gl_Position = screenPos;

	float life = in_position.w;
	//float alpha = 1.0-pow(life,0.5);//sin(life*3.142) * speed;
	float alpha = sin(life*3.142);
	//alpha *= speed;
	alpha = clamp(alpha, 0.0, 1.0);
	//alpha *= 0.2;

	gl_PointSize = mix(8.0, 12.0, in_properties.w);

	//out_color = vec4(vec3(in_properties.z), alpha);
	out_color = vec4(vec3(1.0), alpha);
	out_angle = in_properties.z * 3.142 * 2.0;
} 