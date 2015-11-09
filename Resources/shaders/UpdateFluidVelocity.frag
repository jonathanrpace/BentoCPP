#version 330 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_fluxData;
uniform sampler2D s_mappingData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform float u_textureScrollSpeed;
uniform float u_viscocity;

uniform vec2 u_mousePos;
uniform float u_mouseRadius;
uniform float u_mouseStrength;

// Outputs
layout( location = 0 ) out vec4 out_velocityData;
layout( location = 1 ) out vec4 out_mappingData;

void main(void)
{ 
	ivec2 dimensions = textureSize(s_fluxData, 0);
	vec2 texelSize = 1.0f / dimensions;

	vec4 flux = texture(s_fluxData, in_uv);
	vec4 fluxL = texture(s_fluxData, in_uv - vec2(texelSize.x,0.0f));
	vec4 fluxR = texture(s_fluxData, in_uv + vec2(texelSize.x,0.0f));
	vec4 fluxU = texture(s_fluxData, in_uv - vec2(0.0f,texelSize.y));
	vec4 fluxD = texture(s_fluxData, in_uv + vec2(0.0f,texelSize.y));

	vec4 velocity = vec4(0.0f);
	velocity.x = ((fluxL.y - flux.x) + (flux.y - fluxR.x)) * 0.5;
	velocity.y = ((fluxU.w - flux.z) + (flux.w - fluxD.z)) * 0.5;

	vec4 height = texture(s_heightData, in_uv);
	vec4 heightL = texture(s_heightData, in_uv - vec2(texelSize.x,0.0f));
	vec4 heightR = texture(s_heightData, in_uv + vec2(texelSize.x,0.0f));
	vec4 heightU = texture(s_heightData, in_uv - vec2(0.0f,texelSize.y));
	vec4 heightD = texture(s_heightData, in_uv + vec2(0.0f,texelSize.y));

	/*
	velocity.x -= (height.x+height.y)-(heightR.x+heightR.y);
	velocity.x += (height.x+height.y)-(heightL.x+heightL.y);
	velocity.y -= (height.x+height.y)-(heightD.x+heightD.y);
	velocity.y += (height.x+height.y)-(heightU.x+heightU.y);
	//velocity *= 0.99f;
	*/

	vec4 mappingData = texture(s_mappingData, in_uv);
	mappingData.xy -= velocity.xy * u_textureScrollSpeed * u_viscocity;

	/*
	// Reset the mapping on new fluid
	if ( u_mouseStrength > 0.0f )
	{
		float mouseRatio = 1.0f - min(1.0f, length(in_uv-u_mousePos) / u_mouseRadius);
		mouseRatio = pow(mouseRatio, 0.5f);
		mappingData.xy += (vec2(0.0f)-mappingData.xy) * mouseRatio * 0.2f;
	}
	*/

	out_velocityData = velocity;
	out_mappingData = mappingData;
}








