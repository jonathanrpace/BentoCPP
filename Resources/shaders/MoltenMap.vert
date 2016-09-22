#version 330 core

// Inputs
layout(location = 0) in vec3 in_vertexPosition;
layout(location = 1) in vec4 in_particlePosition;
layout(location = 2) in vec4 in_particleProperties;

uniform sampler2D s_smudgeData;

// Outputs
out gl_PerVertex 
{
	vec4 gl_Position;
};

out Varying
{
	float out_color;
};

vec2 rotateBy( vec2 _pt, float _angle )
{
	float cosValue = cos(_angle);
	float sinValue = sin(_angle);
	return vec2( _pt.x * cosValue - _pt.y * sinValue, _pt.x * sinValue + _pt.y * cosValue );
}

void main(void)
{
	vec3 screenPos = in_vertexPosition;

	// Scale the particle geom vertex
	screenPos.xy *= mix(0.05, 0.07, pow(in_particleProperties.w, 2.0));

	// Rotate and squish the geom vertex based on smudge data sample
	vec2 smudgeVector = texture( s_smudgeData, in_particlePosition.xz ).xy;
	float smudgeStrength = length(smudgeVector) + 0.000001;
	vec2 normSmudgeVector = smudgeVector / smudgeStrength;
	float smudgeAngle = atan(-normSmudgeVector.y, normSmudgeVector.x);
	if ( length(smudgeVector) == 0 )
	{
		smudgeAngle = 0.0;
	}
	screenPos.xy = rotateBy( screenPos.xy, smudgeAngle );
	screenPos.x *= 1.0 + min( smudgeStrength * 25.0, 1.2 );
	screenPos.xy = rotateBy( screenPos.xy, -smudgeAngle );

	// Put the geom vertex at the particle's location
	screenPos.xy += (in_particlePosition.xz - 0.5) * 2.0;

	// Push the particle in/out along the z-axis based on life
	float life = in_particlePosition.w;
	float lifeAlpha = sin(life*3.142);
	lifeAlpha *= mix(0.1, 1.0, pow(in_particleProperties.z, 2.0));
	screenPos.z -= (1.0-lifeAlpha);

	gl_Position = vec4( screenPos, 1.0 );
	out_color = screenPos.z;
} 