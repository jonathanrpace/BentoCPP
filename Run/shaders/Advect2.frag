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
	return clamp( v, vec2(-1.0), vec2(1.0) );
}

float getNeighbourAdvectWeight( in vec2 offset, vec2 v )
{
	v = limit(v * u_dt);
	
	vec2 p = abs(offset + v);
	float weight = 1.0 - max( min(p.x, p.y), 0.0 );
	
	return weight;
}

void main()
{
	ivec2 T = ivec2(gl_FragCoord.xy);
	
	vec4 heightDataOld = texelFetch( s_heightData, T, 0 );
	
    vec4 vC  = texelFetch(s_velocityTexture, T, 0);
	vec2 velocityA = limit(vC.xy * u_dt);
	vec2 velocityB = limit(vC.zw * u_dt);
	
	vec4 heightDataNewA = texture(s_heightData, in_uv - velocityA);
	vec4 heightDataNewB = texture(s_heightData, in_uv - velocityB);
	
	float weightC  = getNeighbourAdvectWeight(vec2( 0,  0), texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0,  0)).xy);
	float weightN  = getNeighbourAdvectWeight(vec2( 0, -1), texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0, -1)).xy);
	float weightS  = getNeighbourAdvectWeight(vec2( 0,  1), texelFetchOffset(s_velocityTexture, T, 0, ivec2( 0,  1)).xy);
	float weightE  = getNeighbourAdvectWeight(vec2( 1,  0), texelFetchOffset(s_velocityTexture, T, 0, ivec2( 1,  0)).xy);
	float weightW  = getNeighbourAdvectWeight(vec2(-1,  0), texelFetchOffset(s_velocityTexture, T, 0, ivec2(-1,  0)).xy);
	float weightNW = getNeighbourAdvectWeight(vec2(-1, -1), texelFetchOffset(s_velocityTexture, T, 0, ivec2(-1, -1)).xy);
	float weightNE = getNeighbourAdvectWeight(vec2( 1, -1), texelFetchOffset(s_velocityTexture, T, 0, ivec2( 1, -1)).xy);
	float weightSW = getNeighbourAdvectWeight(vec2(-1,  1), texelFetchOffset(s_velocityTexture, T, 0, ivec2(-1,  1)).xy);
	float weightSE = getNeighbourAdvectWeight(vec2( 1,  1), texelFetchOffset(s_velocityTexture, T, 0, ivec2( 1,  1)).xy);
	
	float totalRemoved = weightC + weightN + weightS + weightE + weightW + weightNW + weightNE + weightSW + weightSE;
	totalRemoved /= 9.0;
	
	totalRemoved = 1.0 - totalRemoved;
	
	float moltenHeight = heightDataOld.y * totalRemoved;
	
	moltenHeight += heightDataNewA.y / 1.8;
	
	
	moltenHeight = max(0.0, moltenHeight);
	
	
    out_heightData = vec4( heightDataOld.x, moltenHeight, heightDataOld.z, heightDataNewB.w );
}