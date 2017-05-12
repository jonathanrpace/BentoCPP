#version 330 core

// Samplers
uniform sampler2DRect s_tex;

// Inputs
in Varying
{
	vec2 in_uv;
};

// Outputs
layout( location = 0 ) out vec4 out_fragColor;

void main(void)
{ 
	ivec2 texSize = textureSize( s_tex, 0 );

	vec2 texelSize =  vec2(1.0) / texSize;

	vec2 uv = in_uv;
	uv *= texSize;
	uv += vec2(0.5);

	float r = 1.5;

	vec4 sample0 = texture2DRect( s_tex, uv ) * 0.115788;

	vec4 sample1 = texture2DRect( s_tex, uv + vec2( r,  0.0) ) * 0.112244;
	vec4 sample2 = texture2DRect( s_tex, uv + vec2(-r,  0.0) ) * 0.112244;
	vec4 sample3 = texture2DRect( s_tex, uv + vec2( 0.0,  r) ) * 0.112244;
	vec4 sample4 = texture2DRect( s_tex, uv + vec2( 0.0, -r) ) * 0.112244;

	vec4 sample5 = texture2DRect( s_tex, uv + vec2(-r, -r) ) * 0.108808;
	vec4 sample6 = texture2DRect( s_tex, uv + vec2( r, -r) ) * 0.108808;
	vec4 sample7 = texture2DRect( s_tex, uv + vec2(-r,  r) ) * 0.108808;
	vec4 sample8 = texture2DRect( s_tex, uv + vec2( r, -r) ) * 0.108808;

	out_fragColor = sample0 + sample1 + sample2 + sample3 + sample4;
}