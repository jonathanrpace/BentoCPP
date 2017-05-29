#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Uniforms
uniform float u_baseHeight = 0.1;
uniform float u_blendWidth = 0.001;

// Varying
in Varying
{
	vec3 in_worldPosition;
	vec4 in_viewPosition;
	vec3 in_uv;
	vec4 in_heightData;
};

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_viewPosition;
layout( location = 1 ) out vec4 out_worldNormal;
layout( location = 2 ) out vec4 out_forward;

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////
void main(void)
{
	out_worldNormal = vec4(0.0);
	out_viewPosition = in_viewPosition;

	float rockHeight = in_heightData.x;
	float moltenHeight = in_heightData.y;
	float dirtHeight = in_heightData.z;
	float waterHeight = in_heightData.w;

	//float height = u_baseHeight + in_worldPosition.y;

	float fragYPos = u_baseHeight + in_worldPosition.y;

	float rockMin = u_baseHeight*0.5;
	float dirtMin = u_baseHeight + rockHeight;
	float moltenMin = u_baseHeight + rockHeight + dirtHeight;
	float waterMin = u_baseHeight + rockHeight + dirtHeight + moltenHeight;

	float rockAlpha = smoothstep( rockMin - u_blendWidth, rockMin + u_blendWidth, fragYPos );
	float dirtAlpha = smoothstep( dirtMin - u_blendWidth, dirtMin + u_blendWidth, fragYPos );
	float moltenAlpha = smoothstep( moltenMin - u_blendWidth*4.0, moltenMin + u_blendWidth*4.0, fragYPos );
	float waterAlpha = smoothstep( waterMin - u_blendWidth, waterMin + u_blendWidth, fragYPos );
	
	vec3 outColor = vec3(1.0,0.5,0.0); // magma
	outColor = mix( outColor, vec3(0.01,0.01,0.01), rockAlpha );
	outColor = mix( outColor, vec3(0.15,0.15,0.05), dirtAlpha );
	outColor = mix( outColor, vec3(1.0,0.4,0.0), moltenAlpha );
	outColor = mix( outColor, vec3(0.0,0.4,1.0), waterAlpha );
	

	out_forward = vec4( outColor, 1.0 );
}
