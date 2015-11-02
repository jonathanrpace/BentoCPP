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

uniform vec2 u_axis;

// Outputs
layout( location = 0 ) out vec4 out_data0;
layout( location = 1 ) out vec4 out_data1;
layout( location = 2 ) out vec4 out_data2;

vec3 exchange(float t1, float h1, vec2 off)
{
	vec2 uv = in_uv+off;
	float t2 = texture(s_data0, uv).x;
	float h2 = texture(s_data0, uv).y;
	float f1 = t1+h1;
	float f2 = t2+h2;
	float diff = (f2-f1)/2.0;
	diff = clamp(diff*0.0005f, -h1/2.0, h2/2.0);
	return vec3(diff, -off*diff);
}

void main(void)
{ 
	ivec2 dimensions = textureSize( s_data0, 0 );
	vec2 texelSize = 1.0f / dimensions;

	vec4 data0Sample = texture(s_data0, in_uv);
	vec4 data1Sample = texture(s_data1, in_uv);
	vec4 data2Sample = texture(s_data2, in_uv);

	float solidHeight = data0Sample.x;
	float moltenHeight = data0Sample.y;
	vec2 moltenVelocity = data1Sample.xy;

	vec3 result = exchange(solidHeight, moltenHeight,  u_axis * texelSize)
				+ exchange(solidHeight, moltenHeight, -u_axis * texelSize);

	moltenHeight += result.x;
	moltenVelocity += result.yz / texelSize;

	out_data0 = vec4(solidHeight, moltenHeight, 0.0f, 0.0f);
	out_data1 = vec4(moltenVelocity, 0.0f, 0.0f);
	out_data2 = data2Sample;
}