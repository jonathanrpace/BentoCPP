#version 330 core

// Samplers
uniform sampler2D s_heightDataOld;
uniform sampler2D s_heightDataNew;
uniform sampler2D s_mappingData;
uniform sampler2D s_diffuseMap;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform float u_textureScrollSpeed;
uniform float u_viscocity;
uniform float u_mapHeightOffset;
uniform vec2 u_cellSize;

uniform vec2 u_mousePos;
uniform float u_mouseRadius;
uniform float u_mouseStrength;

// Outputs
layout( location = 0 ) out vec4 out_velocityData;
layout( location = 1 ) out vec4 out_mappingData;
layout( location = 2 ) out vec4 out_normal;

void main(void)
{ 
	ivec2 dimensions = textureSize(s_heightDataOld, 0);
	vec2 texelSize = 1.0f / dimensions;

	vec4 height = texture(s_heightDataNew, in_uv);
	vec4 heightL = texture(s_heightDataNew, in_uv - vec2(texelSize.x,0.0f));
	vec4 heightR = texture(s_heightDataNew, in_uv + vec2(texelSize.x,0.0f));
	vec4 heightU = texture(s_heightDataNew, in_uv - vec2(0.0f,texelSize.y));
	vec4 heightD = texture(s_heightDataNew, in_uv + vec2(0.0f,texelSize.y));

	vec4 heightOld = texture(s_heightDataOld, in_uv);
	vec4 heightOldL = texture(s_heightDataOld, in_uv - vec2(texelSize.x,0.0f));
	vec4 heightOldR = texture(s_heightDataOld, in_uv + vec2(texelSize.x,0.0f));
	vec4 heightOldU = texture(s_heightDataOld, in_uv - vec2(0.0f,texelSize.y));
	vec4 heightOldD = texture(s_heightDataOld, in_uv + vec2(0.0f,texelSize.y));

	vec4 velocity = vec4(0.0f);

	velocity.x = ((heightR.x-heightR.y)-(heightOldR.x+heightOldR.y)) - ((heightL.x-heightL.y)-(heightOldL.x+heightOldL.y));
	velocity.y = ((heightD.x-heightD.y)-(heightOldD.x+heightOldD.y)) - ((heightU.x-heightU.y)-(heightOldU.x+heightOldU.y));

	vec4 mappingData = texture(s_mappingData, in_uv);
	vec4 mappingL = texture(s_mappingData, in_uv - vec2(texelSize.x,0.0f));
	vec4 mappingR = texture(s_mappingData, in_uv + vec2(texelSize.x,0.0f));
	vec4 mappingU = texture(s_mappingData, in_uv - vec2(0.0f,texelSize.y));
	vec4 mappingD = texture(s_mappingData, in_uv + vec2(0.0f,texelSize.y));

	mappingData.xy -= velocity.xy * u_textureScrollSpeed * u_viscocity;

	// Reset mapping near mouse
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-u_mousePos) / u_mouseRadius);
	mouseRatio *= u_mouseStrength;
	if ( mouseRatio > 0.0f )
	{
		mappingData.xy = in_uv;
	}

	vec4 diffuseSample = texture(s_diffuseMap, mappingData.xy);
	vec4 diffuseSampleL = texture(s_diffuseMap, mappingL.xy);
	vec4 diffuseSampleR = texture(s_diffuseMap, mappingR.xy);
	vec4 diffuseSampleU = texture(s_diffuseMap, mappingU.xy);
	vec4 diffuseSampleD = texture(s_diffuseMap, mappingD.xy);

	// Calculate normals
	vec3 va = normalize(vec3(u_cellSize.x, 
	(heightR.x+heightR.y+diffuseSampleR.x*u_mapHeightOffset)-
	(heightL.x+heightL.y+diffuseSampleL.x*u_mapHeightOffset), 0.0f));

    vec3 vb = normalize(vec3(0.0f, 
	(heightD.x+heightD.y+diffuseSampleD.x*u_mapHeightOffset)-
	(heightU.x+heightU.y+diffuseSampleU.x*u_mapHeightOffset), u_cellSize.y));
    vec3 normal = -cross(va,vb);
	
	out_velocityData = velocity;
	out_mappingData = mappingData;
	out_normal = vec4(normal,0.0f);
}








