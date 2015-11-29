#version 330 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_mappingData;
uniform sampler2D s_diffuseMap;
uniform sampler2D s_fluxData;

// Inputs
in Varying
{
	vec2 in_uv;
};

uniform float u_textureScrollSpeed;
uniform float u_mapHeightOffset;
uniform float u_velocityScalar;
uniform vec2 u_cellSize;
uniform float u_heatViscosityPower;

uniform vec2 u_mousePos;
uniform float u_mouseRadius;
uniform float u_mouseStrength;

// Outputs
layout( location = 0 ) out vec4 out_velocityData;
layout( location = 1 ) out vec4 out_mappingData;
layout( location = 2 ) out vec4 out_normal;

void main(void)
{ 
	ivec2 dimensions = textureSize(s_mappingData, 0);
	vec2 texelSize = 1.0f / dimensions;

	vec4 heightSample = texture(s_heightData, in_uv);
	vec4 heightSampleL = texture(s_heightData, in_uv - vec2(texelSize.x,0.0f));
	vec4 heightSampleR = texture(s_heightData, in_uv + vec2(texelSize.x,0.0f));
	vec4 heightSampleU = texture(s_heightData, in_uv - vec2(0.0f,texelSize.y));
	vec4 heightSampleD = texture(s_heightData, in_uv + vec2(0.0f,texelSize.y));

	vec4 flux = texture(s_fluxData, in_uv);
	vec4 fluxL = texture(s_fluxData, in_uv - vec2(texelSize.x,0.0f));
	vec4 fluxR = texture(s_fluxData, in_uv + vec2(texelSize.x,0.0f));
	vec4 fluxU = texture(s_fluxData, in_uv - vec2(0.0f,texelSize.y));
	vec4 fluxD = texture(s_fluxData, in_uv + vec2(0.0f,texelSize.y));

	float heat = heightSample.z;
	float viscosity = pow(heat, u_heatViscosityPower);

	vec4 velocity = vec4(0.0f);
	velocity.x = -(fluxL.y - fluxR.x);
	velocity.y = -(fluxU.w - fluxD.z);
	velocity.xy *= -u_velocityScalar * viscosity;

	vec4 heightDiffs = vec4(0.0f);
	heightDiffs.x = (heightSampleL.x+heightSampleL.y) - (heightSample.x+heightSample.y);
	heightDiffs.y = (heightSampleR.x+heightSampleR.y) - (heightSample.x+heightSample.y);
	heightDiffs.z = (heightSampleU.x+heightSampleU.y) - (heightSample.x+heightSample.y);
	heightDiffs.w = (heightSampleD.x+heightSampleD.y) - (heightSample.x+heightSample.y);

	vec4 h = vec4(0.0f);
	h.x = length( vec2(heightDiffs.x, u_cellSize.x) );
	h.y = length( vec2(heightDiffs.y, u_cellSize.x) );
	h.z = length( vec2(heightDiffs.z, u_cellSize.y) );
	h.w = length( vec2(heightDiffs.w, u_cellSize.y) );
	velocity.xy /= 1.0f + (h.x+h.y+h.z+h.w);


	vec4 mappingData = texture(s_mappingData, in_uv);
	vec4 mappingL = texture(s_mappingData, in_uv - vec2(texelSize.x,0.0f));
	vec4 mappingR = texture(s_mappingData, in_uv + vec2(texelSize.x,0.0f));
	vec4 mappingU = texture(s_mappingData, in_uv - vec2(0.0f,texelSize.y));
	vec4 mappingD = texture(s_mappingData, in_uv + vec2(0.0f,texelSize.y));

	mappingData.xy -= velocity.xy * u_textureScrollSpeed;

	// Reset mapping near mouse
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-u_mousePos) / u_mouseRadius);
	mouseRatio *= u_mouseStrength;
	if ( mouseRatio > 0.0f )
	{
		mappingData.xy = in_uv;
	}

	// Calculate normals
	vec4 diffuseSample = texture(s_diffuseMap, mappingData.xy);
	vec4 diffuseSampleL = texture(s_diffuseMap, mappingL.xy);
	vec4 diffuseSampleR = texture(s_diffuseMap, mappingR.xy);
	vec4 diffuseSampleU = texture(s_diffuseMap, mappingU.xy);
	vec4 diffuseSampleD = texture(s_diffuseMap, mappingD.xy);

	float heightR = heightSampleR.x + heightSampleR.y;
	float heightL = heightSampleL.x + heightSampleL.y;
	float heightU = heightSampleU.x + heightSampleU.y;
	float heightD = heightSampleD.x + heightSampleD.y;
	float height = heightSample.x + heightSample.y;

	float mapHeightOffset = u_mapHeightOffset;// * (1.0f+heat*5.0f);

	vec3 va = normalize(vec3(u_cellSize.x, 
	(heightR+diffuseSampleR.x*mapHeightOffset)-
	(heightL+diffuseSampleL.x*mapHeightOffset), 0.0f));

    vec3 vb = normalize(vec3(0.0f, 
	(heightD+diffuseSampleD.x*mapHeightOffset)-
	(heightU+diffuseSampleU.x*mapHeightOffset), u_cellSize.y));
    vec3 normal = -cross(va,vb);
	
	out_velocityData = velocity;
	out_mappingData = mappingData;
	out_normal = vec4(normal,0.0f);
}








