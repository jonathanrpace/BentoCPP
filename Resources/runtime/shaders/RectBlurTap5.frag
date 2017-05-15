#version 330 core

// Samplers
uniform sampler2DRect s_tex;

uniform float u_offset;

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

	vec2 uv = in_uv;
	uv *= texSize;
	
	vec4 sample0 = texture2DRect( s_tex, uv + vec2(-u_offset, -u_offset) );
	vec4 sample1 = texture2DRect( s_tex, uv + vec2( u_offset, -u_offset) );
	vec4 sample2 = texture2DRect( s_tex, uv + vec2(-u_offset,  u_offset) );
	vec4 sample3 = texture2DRect( s_tex, uv + vec2( u_offset,  u_offset) );
	out_fragColor = (sample0 + sample1 + sample2 + sample3) * 0.25;
	
	/*
	float r = 4.0;
	vec4 sample0 = texture2DRect( s_tex, uv ) * 0.14776131634681883;

	vec4 sample1 = texture2DRect( s_tex, uv + vec2( r,  0.0) ) * 0.11831801270312062;
	vec4 sample2 = texture2DRect( s_tex, uv + vec2(-r,  0.0) ) * 0.11831801270312062;
	vec4 sample3 = texture2DRect( s_tex, uv + vec2( 0.0,  r) ) * 0.11831801270312062;
	vec4 sample4 = texture2DRect( s_tex, uv + vec2( 0.0, -r) ) * 0.11831801270312062;

	vec4 sample5 = texture2DRect( s_tex, uv + vec2(-r, -r) ) * 0.09474165821017469;
	vec4 sample6 = texture2DRect( s_tex, uv + vec2( r, -r) ) * 0.09474165821017469;
	vec4 sample7 = texture2DRect( s_tex, uv + vec2(-r,  r) ) * 0.09474165821017469;
	vec4 sample8 = texture2DRect( s_tex, uv + vec2( r, -r) ) * 0.09474165821017469;

	out_fragColor = sample0 + sample1 + sample2 + sample3 + sample4 + sample5 + sample6 + sample7 + sample8;
	*/

	/*
	vec4 sample0 = texture2DRect( s_tex, uv + vec2( 0.0,  0.0) ) * 0.25;
	vec4 sample1 = texture2DRect( s_tex, uv + vec2(-4.0, -4.0) ) * 0.25;
	vec4 sample2 = texture2DRect( s_tex, uv + vec2( 0.0, -4.0) ) * 0.25;
	vec4 sample3 = texture2DRect( s_tex, uv + vec2(-4.0,  0.0) ) * 0.25;
	

	out_fragColor = sample0 + sample1 + sample2 + sample3;
	*/

	
}