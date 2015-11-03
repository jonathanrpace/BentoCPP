#version 330 core

// Samplers
uniform sampler2D s_data0;
uniform sampler2D s_data1;
uniform sampler2D s_data2;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform float u_dt = 1.0f / 10.f;
uniform float u_cellWidth = 1.0f / 1.0f;	
uniform float u_cellHeight = 1.0f / 1.0f;

uniform vec2 u_mousePos = vec2(0.5f,0.5f);				// In uv coordinates
uniform float u_mouseRadius = 0.025f;

// Outputs
layout( location = 0 ) out vec4 out_data0;
layout( location = 1 ) out vec4 out_data1;
layout( location = 2 ) out vec4 out_data2;

void main(void)
{ 
	ivec2 dimensions = textureSize( s_data0, 0 );
	vec2 texelSize = 1.0f / dimensions;

	vec4 data0Sample = texture(s_data0, in_uv);
	vec4 data1Sample = texture(s_data1, in_uv);
	vec4 data2Sample = texture(s_data2, in_uv);

	float fluidHeight = data0Sample.y;

	vec4 flux = texture(s_data1, in_uv);
	float fluxL = texture(s_data1, in_uv - vec2(texelSize.x,0.0f)).y;
	float fluxR = texture(s_data1, in_uv + vec2(texelSize.x,0.0f)).x;
	float fluxU = texture(s_data1, in_uv - vec2(0.0f,texelSize.y)).w;
	float fluxD = texture(s_data1, in_uv + vec2(0.0f,texelSize.y)).z;
	vec4 nFlux = vec4(fluxL, fluxR, fluxU, fluxD);

	float fluxChange = u_dt * ((nFlux.x+nFlux.y+nFlux.z+nFlux.w)-(flux.x+flux.y+flux.z+flux.w));
	float newFluidHeight = fluidHeight + (fluxChange/(u_cellWidth*u_cellHeight));


	// Add some fluid near the mouse
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-u_mousePos) / u_mouseRadius);
	newFluidHeight += mouseRatio * 0.005;

	//out_data0 = data0Sample;
	out_data1 = data1Sample;
	out_data2 = data2Sample;

	out_data0 = vec4(data0Sample.x, newFluidHeight, data0Sample.zw);
}








