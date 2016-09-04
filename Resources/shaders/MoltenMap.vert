#version 330 core

// Inputs
layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_props;
layout(location = 2) in vec4 in_properties;

uniform sampler2D s_velocityData;

// Outputs
out gl_PerVertex 
{
	vec4 gl_Position;
	float gl_PointSize;
};

out Varying
{
	float out_color;
	vec2 out_smudgeVector;
	float out_angleOffset;
};

void main(void)
{
	vec4 velocityData = texture(s_velocityData, in_position.xz);

	float speed = length(velocityData.xy) * 20.0;

	vec4 screenPos = vec4(in_position.x, in_position.z, 1.0, 1.0);
	screenPos.xy -= 0.5;
	screenPos.xy *= 2.0;
	gl_Position = screenPos;
	gl_PointSize = mix(24.0, 64.0, pow(in_properties.w, 1.5));

	float life = in_position.w;
	float lifeAlpha = sin(life*3.142);

	lifeAlpha *= mix(0.5, 1.0, in_properties.z);

	//alpha = clamp(alpha, 0.0, 1.0);

	vec2 smudgeVector = in_props.xy;

	out_color = lifeAlpha;
	out_smudgeVector = smudgeVector;
	out_angleOffset = 0.0;//mix(0.0, 3.142, in_properties.w);
} 