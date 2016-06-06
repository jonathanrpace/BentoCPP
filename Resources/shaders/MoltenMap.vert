#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_properties;

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
	vec4 screenPos = vec4(in_position.x, in_position.z, 1.0, 1.0);
	screenPos.xy -= 0.5;
	screenPos.xy *= 2.0;
	gl_Position = screenPos;

	float life = in_position.w;
	float alpha = sin(life*3.142);

	gl_PointSize = mix(12.0, 20.0, in_properties.w);

	out_color = vec4(vec3(1.0), alpha);
	out_angle = in_properties.z * 3.142 * 2.0;
} 