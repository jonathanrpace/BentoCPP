#version 330 core

// Samplers
uniform sampler2DRect s_colorBuffer;
uniform sampler2DRect s_blurredColorBuffer;

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
	vec4 colorSample = texture2DRect( s_colorBuffer, in_uv * textureSize(s_colorBuffer) );
	vec4 blurredColorSample = texture2DRect( s_blurredColorBuffer, in_uv * textureSize(s_blurredColorBuffer) );

	vec4 bloom = blurredColorSample;

	bloom = pow(bloom, vec4(u_bloomPow));
	bloom *= u_bloomStrength;

	out_fragColor = colorSample + bloom;
}