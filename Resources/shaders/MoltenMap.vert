#version 330 core

// Inputs
layout(location = 0) in vec3 in_vertexPosition;
layout(location = 1) in vec4 in_particlePosition;
layout(location = 2) in vec4 in_particleProps;
layout(location = 3) in vec4 in_particleProperties;

uniform sampler2D s_velocityData;

// Outputs
out gl_PerVertex 
{
	vec4 gl_Position;
};

out Varying
{
	float out_color;
	//vec2 out_smudgeVector;
	//float out_angleOffset;
};

vec2 rotateBy( vec2 _pt, float _angle )
{
	float cosValue = cos(_angle);
	float sinValue = sin(_angle);
	return vec2( _pt.x * cosValue - _pt.y * sinValue, _pt.x * sinValue + _pt.y * cosValue );
}

void main(void)
{
	vec4 velocityData = texture(s_velocityData, in_particlePosition.xz);
	float speed = length(velocityData.xy) * 20.0;

	float life = in_particlePosition.w;
	float lifeAlpha = sin(life*3.142);
	lifeAlpha *= mix(0.5, 1.0, in_particleProperties.z);

	vec3 screenPos = in_vertexPosition;
	screenPos.xy *= mix(0.1, 0.03, in_particleProperties.w);
	screenPos.xy += (in_particlePosition.xz - 0.5) * 2.0;
	screenPos.z -= (1.0-lifeAlpha);

	//vec4 screenPos = vec4(in_particlePosition.x, in_particlePosition.z, 0.0, 1.0);
	//screenPos.xy -= 0.5;
	//screenPos.xy *= 2.0;
	//screenPos.xyz += in_vertexPosition.xyz;

	gl_Position = vec4( screenPos, 1.0 );

	

	//vec2 smudgeVector = in_props.xy;


	out_color = screenPos.z;
	//out_smudgeVector = smudgeVector;
	//out_angleOffset = 0.0;
} 