#version 330 core

// Outputs
layout( location = 0 ) out vec4 out_fragColor;

in Varying
{
	vec4 in_color;
	float in_angle;
	float in_life;
	float in_alpha;
	float in_offset;
};

uniform sampler2D s_texture;

void main(void)
{
	vec2 uv = gl_PointCoord;
	
	vec2 rotatedUV = uv;
	rotatedUV -= vec2(0.5);
	rotatedUV = vec2( rotatedUV.x * cos(in_angle) - rotatedUV.y * sin(in_angle),
					  rotatedUV.x * sin(in_angle) + rotatedUV.y * cos(in_angle) );
	rotatedUV += vec2(0.5);

	vec4 textureSampleA = texture(s_texture, rotatedUV);
	float textureAlpha = textureSampleA.x;

	float alpha = (textureAlpha - (1.0-in_color.w) * 0.5) * in_color.w;

	if ( alpha <= 0.0 )
		discard;

	vec2 offsetUV = uv;
	offsetUV -= vec2(-in_offset * 0.5, in_offset);
	vec4 textureSampleB = texture(s_texture, offsetUV);
	alpha *= textureSampleB.y;

	alpha *= in_alpha;

	out_fragColor = vec4(textureSampleB.yyy * 0.75 * (1.0-uv.yyy), alpha);
} 