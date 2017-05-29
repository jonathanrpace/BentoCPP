#version 330 core

// Samplers
uniform sampler2DRect s_colorBuffer;
uniform sampler2DRect s_blurredColorBuffer;
uniform sampler2D s_dirtyLens;

uniform float u_bloomPow = 2.0;
uniform float u_bloomStrength = 1.0;

// Inputs
in Varying
{
	vec2 in_uv;
};

// Outputs
layout( location = 0 ) out vec4 out_fragColor;

void main(void)
{ 
	vec2 backBufferSize = vec2( textureSize(s_colorBuffer) );

	vec4 colorSample = texture2DRect( s_colorBuffer, in_uv * backBufferSize );

	float aspectRatio = backBufferSize.y / backBufferSize.x;
	vec2 stretchedUV = vec2( in_uv.x, in_uv.y * aspectRatio );

	vec4 dirtSample = texture( s_dirtyLens, stretchedUV );

	vec2 bloomUV = dirtSample.rg;
	bloomUV.y += floor(stretchedUV.y);
	bloomUV.y /= aspectRatio;
	
	bloomUV *= textureSize(s_blurredColorBuffer);
	//bloomUV = in_uv * textureSize(s_blurredColorBuffer);
	vec4 blurredColorSample = texture2DRect( s_blurredColorBuffer, bloomUV );

	vec4 bloom = blurredColorSample;

	bloom = pow(bloom, vec4(u_bloomPow));
	bloom *= u_bloomStrength * colorSample.a;

	float vignetteDelta = length(in_uv - 0.5);
	vignetteDelta = pow( vignetteDelta, 5.0 );

	float dirtAlpha = mix( 0.005, dirtSample.b, vignetteDelta );

	bloom *= dirtAlpha;

	colorSample *= mix( 1.0, 0.5, vignetteDelta );

	out_fragColor = colorSample + bloom;
}