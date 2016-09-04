#version 330 core

// Outputs
layout( location = 0 ) out vec4 out_fragColor;
out float gl_FragDepth;

// Samplers

uniform sampler2D s_texture;

in Varying
{
	float in_color;
	vec2 in_smudgeVector;
	float in_angleOffset;
};

vec2 rotateBy( vec2 _pt, float _angle )
{
	float cosValue = cos(_angle);
	float sinValue = sin(_angle);

	return vec2( _pt.x * cosValue - _pt.y * sinValue, _pt.x * sinValue + _pt.y * cosValue );
}

void main(void)
{
	if ( in_color < 0.0001 )
		discard;


	float smudgeStrength = length(in_smudgeVector) + 0.000001;
	vec2 normSmudgeVector = in_smudgeVector / smudgeStrength;
	float smudgeAngle = atan(normSmudgeVector.y, normSmudgeVector.x);

	vec2 uv = gl_PointCoord - vec2(0.5);
	uv = rotateBy( uv, smudgeAngle);
	//uv *= vec2(1.0) + clamp( in_smudgeVector.yx * vec2(5.0), vec2(0.0), vec2(5.0) );
	uv.x *= 1.0 + min( smudgeStrength * 25.0, 2.0 );
	uv = rotateBy( uv, -smudgeAngle );
	uv += vec2(0.5);

	uv = clamp(uv, vec2(0.0), vec2(1.0));

	vec4 textureSample = texture(s_texture, uv);
	float alpha = textureSample.x;
	alpha *= in_color;

	gl_FragDepth = alpha;

	out_fragColor = vec4(vec3(alpha), 1.0);
} 