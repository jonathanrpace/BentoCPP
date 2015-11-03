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
uniform float u_pipeSize = 1.0f;						// Cross sectional area of pipe
uniform float u_gravity = 1.0f;
uniform float u_pipeLength = 1.0f;// / 32.0f;
uniform float u_cellWidth = 1.0f / 1.0f;	
uniform float u_cellHeight = 1.0f / 1.0f;

// Outputs
layout( location = 0 ) out vec4 out_data0;
layout( location = 1 ) out vec4 out_data1;
layout( location = 2 ) out vec4 out_data2;
/*
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
*/
void main(void)
{ 
	ivec2 dimensions = textureSize( s_data0, 0 );
	vec2 texelSize = 1.0f / dimensions;

	vec4 data0Sample = texture(s_data0, in_uv);
	vec4 data1Sample = texture(s_data1, in_uv);
	vec4 data2Sample = texture(s_data2, in_uv);

	/*
	

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
	*/

	vec4 data0SampleL = texture(s_data0, in_uv - vec2(texelSize.x,0.0f));
	vec4 data0SampleR = texture(s_data0, in_uv + vec2(texelSize.x,0.0f));
	vec4 data0SampleU = texture(s_data0, in_uv - vec2(0.0f,texelSize.y));
	vec4 data0SampleD = texture(s_data0, in_uv + vec2(0.0f,texelSize.y));

	vec4 data1SampleL = texture(s_data1, in_uv - vec2(texelSize.x,0.0f));
	vec4 data1SampleR = texture(s_data1, in_uv + vec2(texelSize.x,0.0f));
	vec4 data1SampleU = texture(s_data1, in_uv - vec2(0.0f,texelSize.y));
	vec4 data1SampleD = texture(s_data1, in_uv + vec2(0.0f,texelSize.y));

	vec4 terrainHeight = vec4(data0Sample.x);
	vec4 nTerrainHeight = vec4(data0SampleL.x, data0SampleR.x, data0SampleU.x, data0SampleD.x);
	vec4 fluidHeight = vec4(data0Sample.y);
	vec4 nFluidHeight = vec4(data0SampleL.y, data0SampleR.y, data0SampleU.y, data0SampleD.y);
	vec4 flux = data1Sample;//vec4(data1SampleL.y, data1SampleR.x, data1SampleU.w, data1SampleD.z);


	vec4 height = terrainHeight + fluidHeight;
	vec4 nHeight = nTerrainHeight + nFluidHeight;
	vec4 heightDiff = height - nHeight;
	vec4 potentialChange = u_pipeSize * ((u_gravity * heightDiff) / u_pipeLength);

	vec4 newFlux = max(vec4(0.0f), flux + u_dt * potentialChange);

	// Need to scale down the new flux so that we can't drain more fluid than we have this step
	float limit = min(1.0f, (fluidHeight.x * u_cellWidth * u_cellHeight) / (newFlux.x + newFlux.y + newFlux.z + newFlux.w) );
	newFlux *= limit;


	out_data0 = data0Sample;
	//out_data1 = data1Sample;
	out_data2 = data2Sample;

	out_data1 = newFlux;


	/*

	// In a seperate pass, update the water heights based on the flux passe's output
	float influxL = texture(fluxData, uv(x-1,y));
	float influxR = texture(fluxData, uv(x+1,y));
	float influxU = texture(fluxData, uv(x,y-1));
	float influxD = texture(fluxData, uv(x,y+1));

	vec4 influx = vec4(influxL, influxR, influxU, influxD);
	vec4 outflux = texture(fluxData, uv);

	float fluxChange = dt * (influx - outflux);
	float newHeight = currHeight + (fluxChange/(cellWidth*cellHeight));
	// Write new height to data
	*/
}








