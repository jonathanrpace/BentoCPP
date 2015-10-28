#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

// Textures
uniform sampler2D s_textureData0;
uniform sampler2D s_textureData1;
uniform sampler2D s_textureData2;
 
// Uniforms
uniform mat4 u_mvpMatrix;
uniform mat4 u_modelMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_normalModelViewMatrix;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

// GL
out gl_PerVertex 
{
	vec4 gl_Position;
};

// Varying
out Varying
{
	vec3 out_viewNormal;
	vec4 out_viewPosition;
};

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

void main(void)
{
	// Pluck some values out of the texture data
	vec4 textureDataSample0 = texture(s_textureData0, in_uv);
	float solidHeight = textureDataSample0.x;
	float moltenHeight = textureDataSample0.y;

	vec4 position = vec4(in_position, 1.0f);
	position.y += solidHeight;
	position.y += moltenHeight;
	
	out_viewNormal = vec3(0.0f, 1.0f, 0.0f);
	out_viewPosition = u_modelViewMatrix * position;

	gl_Position = u_mvpMatrix * position;
} 