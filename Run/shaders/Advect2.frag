#version 330 core

// From VS
in Varying
{
	vec2 in_uv;
};

layout( location = 0 ) out vec4 out_heightData;

uniform sampler2D s_velocityTexture;
uniform sampler2D s_heightData;
uniform sampler2D s_pressure;

uniform float u_dt;
uniform float u_dissipation;

vec2 limit( vec2 v )
{
	return v;
	float vm = length(v);
	v /= (1.0 + max(0.0, vm-1.0));
	
	return v;
}

float getNeighbourAdvectWeight( in vec2 offset, vec2 v )
{
	v *= u_dt;
	vec2 p = abs(offset - v);
	p = vec2(1.0) - p;
	float weight = max(p.x, p.y);
	weight = clamp( weight, 0.0, 1.0 );

	return weight;
}

void main()
{
	ivec2 T = ivec2(gl_FragCoord.xy);
	
	float cellSize = 1.0;// / textureSize( s_velocityTexture, 0 ).x;
	
	vec4 heightDataOld = texelFetch( s_heightData, T, 0 );
	
    float pC  = texelFetch(s_pressure, T, 0).x;
	
    vec4 vC  = texelFetch(s_velocityTexture, T, 0);
	vec2 velocityA = limit(vC.xy * u_dt);
	vec2 velocityB = limit(vC.zw * u_dt);
	
	vec4 heightDataNewA = texture(s_heightData, in_uv - velocityA * cellSize) / 8.0;
	vec4 heightDataNewB = texture(s_heightData, in_uv - velocityB * cellSize);
	
	/*
	vec4 vN  = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0, -1));
	vec4 vS  = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0,  1));
	vec4 vE  = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 1,  0));
	vec4 vW  = texelFetchOffset(s_velocityTexture, T, 0, ivec2(-1,  0));
	vec4 vNW = texelFetchOffset(s_velocityTexture, T, 0, ivec2(-1, -1));
	vec4 vNE = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 1, -1));
	vec4 vSW = texelFetchOffset(s_velocityTexture, T, 0, ivec2(-1,  1));
	vec4 vSE = texelFetchOffset(s_velocityTexture, T, 0, ivec2( 1,  1));
	*/
	
	
	float weightN  = getNeighbourAdvectWeight(vec2( 0, -1), texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0, -1)).xy);
	float weightS  = getNeighbourAdvectWeight(vec2( 0,  1), texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0,  1)).xy);
	float weightE  = getNeighbourAdvectWeight(vec2( 1,  0), texelFetchOffset(s_velocityTexture, T, 0, ivec2( 1,  0)).xy);
	float weightW  = getNeighbourAdvectWeight(vec2(-1,  0), texelFetchOffset(s_velocityTexture, T, 0, ivec2(-1,  0)).xy);
	float weightNW = getNeighbourAdvectWeight(vec2(-1, -1), texelFetchOffset(s_velocityTexture, T, 0, ivec2(-1, -1)).xy);
	float weightNE = getNeighbourAdvectWeight(vec2( 1, -1), texelFetchOffset(s_velocityTexture, T, 0, ivec2( 1, -1)).xy);
	float weightSW = getNeighbourAdvectWeight(vec2(-1,  1), texelFetchOffset(s_velocityTexture, T, 0, ivec2(-1,  1)).xy);
	float weightSE = getNeighbourAdvectWeight(vec2( 1,  1), texelFetchOffset(s_velocityTexture, T, 0, ivec2( 1,  1)).xy);
	
	float totalRemoved = weightN + weightS + weightE + weightW + weightNW + weightNE + weightSW + weightSE;
	totalRemoved /= 9.0;
	totalRemoved *= heightDataOld.y;
	
	float moltenHeight = heightDataOld.y;
	moltenHeight = heightDataNewA.y;// / 9.0;

	//moltenHeight -= totalRemoved;
	//moltenHeight = max(0.0, moltenHeight);
	
	
    out_heightData = vec4( heightDataOld.x, moltenHeight, heightDataOld.z, heightDataNewB.w );
}