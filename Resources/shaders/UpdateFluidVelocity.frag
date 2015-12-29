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

uniform float u_viscosityMin;
uniform float u_viscosityMax;

uniform float u_heatViscosityPower;
uniform float u_heatViscosityBias;

uniform vec2 u_cellSize;

uniform vec2 u_mousePos;
uniform float u_mouseRadius;
uniform float u_mouseVolumeStrength;

uniform int u_numHeightMips;

// Outputs
layout( location = 0 ) out vec4 out_velocityData;
layout( location = 1 ) out vec4 out_mappingData;
layout( location = 2 ) out vec4 out_normal;

float CalcViscosity( float _heat, float _viscosityScalar )
{
	float viscosity = mix( u_viscosityMin, u_viscosityMax, _viscosityScalar );
	return pow(smoothstep( 0.0f, 1.0f, clamp(_heat-u_heatViscosityBias, 0.0f, 1.0f)), u_heatViscosityPower) * viscosity;
}

void main(void)
{
	ivec2 dimensions = textureSize( s_heightData, 0 );
	vec2 texelSize = 1.0f / dimensions;

	ivec2 texelCoordC = ivec2(gl_FragCoord.xy);
	ivec2 texelCoordL = texelCoordC - ivec2(1,0);
	ivec2 texelCoordR = texelCoordC + ivec2(1,0);
	ivec2 texelCoordU = texelCoordC - ivec2(0,1);
	ivec2 texelCoordD = texelCoordC + ivec2(0,1);

	vec4 heightDataC = texelFetch(s_heightData, texelCoordC, 0);
	vec4 heightDataL = texelFetch(s_heightData, texelCoordL, 0);
	vec4 heightDataR = texelFetch(s_heightData, texelCoordR, 0);
	vec4 heightDataU = texelFetch(s_heightData, texelCoordU, 0);
	vec4 heightDataD = texelFetch(s_heightData, texelCoordD, 0);

	vec4 mappingDataC = texelFetch(s_mappingData, texelCoordC, 0);
	vec4 mappingDataL = texelFetch(s_mappingData, texelCoordL, 0);
	vec4 mappingDataR = texelFetch(s_mappingData, texelCoordR, 0);
	vec4 mappingDataU = texelFetch(s_mappingData, texelCoordU, 0);
	vec4 mappingDataD = texelFetch(s_mappingData, texelCoordD, 0);

	vec4  fluxC = texelFetch(s_fluxData, texelCoordC, 0);
	float fluxL = texelFetch(s_fluxData, texelCoordL, 0).y;
	float fluxR = texelFetch(s_fluxData, texelCoordR, 0).x;
	float fluxU = texelFetch(s_fluxData, texelCoordU, 0).w;
	float fluxD = texelFetch(s_fluxData, texelCoordD, 0).z;

	vec4 diffuseSampleC = texture(s_diffuseMap, mappingDataC.xy);
	vec4 diffuseSampleL = texture(s_diffuseMap, mappingDataL.xy);
	vec4 diffuseSampleR = texture(s_diffuseMap, mappingDataR.xy);
	vec4 diffuseSampleU = texture(s_diffuseMap, mappingDataU.xy);
	vec4 diffuseSampleD = texture(s_diffuseMap, mappingDataD.xy);

	float heat = heightDataC.z;
	float viscosity = CalcViscosity(heat, diffuseSampleC.y);

	// Calculate velocity from flux
	vec4 velocity = vec4(0.0f);
	velocity.x = ((fluxL + fluxC.y) - (fluxR + fluxC.x)) / texelSize.x;
	velocity.y = ((fluxU + fluxC.w) - (fluxD + fluxC.z)) / texelSize.y;
	velocity.xy *= u_velocityScalar * viscosity;

	vec4 heightDiffs = vec4(0.0f);
	heightDiffs.x = (heightDataL.x+heightDataL.y) - (heightDataC.x+heightDataC.y);
	heightDiffs.y = (heightDataR.x+heightDataR.y) - (heightDataC.x+heightDataC.y);
	heightDiffs.z = (heightDataU.x+heightDataU.y) - (heightDataC.x+heightDataC.y);
	heightDiffs.w = (heightDataD.x+heightDataD.y) - (heightDataC.x+heightDataC.y);

	vec4 h = vec4(0.0f);
	h.x = length( vec2(heightDiffs.x, u_cellSize.x) );
	h.y = length( vec2(heightDiffs.y, u_cellSize.x) );
	h.z = length( vec2(heightDiffs.z, u_cellSize.y) );
	h.w = length( vec2(heightDiffs.w, u_cellSize.y) );
	velocity.xy *= (1.0f + (h.x+h.y+h.z+h.w));
	
	// Update mappingData from velocity
	mappingDataC.xy -= (velocity.xy * u_textureScrollSpeed * texelSize);

	// Calculate normals
	float heightR = heightDataR.x + heightDataR.y;
	float heightL = heightDataL.x + heightDataL.y;
	float heightU = heightDataU.x + heightDataU.y;
	float heightD = heightDataD.x + heightDataD.y;
	float heightC = heightDataC.x + heightDataC.y;

	vec3 va = normalize(vec3(u_cellSize.x, 
	(heightR+diffuseSampleR.x*u_mapHeightOffset)-
	(heightL+diffuseSampleL.x*u_mapHeightOffset), 0.0f));

    vec3 vb = normalize(vec3(0.0f, 
	(heightD+diffuseSampleD.x*u_mapHeightOffset)-
	(heightU+diffuseSampleU.x*u_mapHeightOffset), u_cellSize.y));
    vec3 normal = -cross(va,vb);

	// Calculate occlusion
	float occlusion = 0.0f;
	for ( int i = 1; i < u_numHeightMips; i++ )
	{
		vec4 mippedHeightDataC = textureLod(s_heightData, in_uv, float(i));
		float mippedHeight = mippedHeightDataC.x + mippedHeightDataC.y;
		float diff = max(0.0f, mippedHeight - heightC);
		float ratio = diff / u_cellSize.x;
		float angle = atan(ratio);
		const float PI = 3.14159265359;
		const float HALF_PI = PI * 0.5f;
		float occlusionFoThisMip = angle / HALF_PI;

		occlusion += occlusionFoThisMip;
	}
	occlusion /= u_numHeightMips;

	
	// Output
	out_velocityData = velocity;
	out_mappingData = mappingDataC;
	out_normal = vec4(normal,occlusion);
}








