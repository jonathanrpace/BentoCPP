#version 330 core

// Inputs
layout(location = 0) in vec4 in_data0;
layout(location = 1) in vec4 in_data1;
layout(location = 2) in vec4 in_data2;
layout(location = 3) in vec4 in_properties;

// Uniforms
//uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_projMatrix;
uniform float u_terrainSize = 1.0;
uniform vec3 u_cameraPos;
uniform float u_fadeInTime;
uniform float u_fadeOutTime;
uniform ivec2 u_frameBufferSize;
//uniform vec3 u_lightDir;

//const float PI = 3.14159265359;
//const float PI_2 = PI * 2.0;

out float out_lifeNrm;
out float out_alpha;
out vec3 out_viewPos;
out vec3 out_worldPos;
out float out_alive;

void main(void)
{
	// Grab some values from the attributes
	vec3 position = in_data0.xyz;
	float size = in_data0.w;
	float lifeNrm = in_data2.y;
	
	// Position
	position.xz *= u_terrainSize;
	position.xz -= u_terrainSize*0.5;

	// Alpha
	float alpha = 1.0;
	alpha *= min(1.0, lifeNrm / u_fadeInTime);
	alpha *= min(1.0, (1.0-lifeNrm) / u_fadeOutTime);
	out_alpha = alpha;

	// Life
	out_lifeNrm = lifeNrm;

	float alive = in_data2.z;

	// Point size
	vec4 viewPos = u_modelViewMatrix * vec4(position, 1.0);
	vec4 projCorner = u_projMatrix * vec4(size, size, viewPos.z, viewPos.w);
	float pointSize = u_frameBufferSize.y * projCorner.x / projCorner.w;
	gl_PointSize = alive > 0.5 ? pointSize : 0.0f;
	gl_Position = u_projMatrix * viewPos;
	out_viewPos = viewPos.xyz;
	out_worldPos = position.xyz;

	out_alive = alive;
}