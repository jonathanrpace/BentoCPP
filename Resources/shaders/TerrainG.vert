#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

// Uniforms
uniform mat4 u_viewMatrix;


uniform vec3 u_fogColorAway;
uniform vec3 u_fogColorTowards;
uniform float u_fogHeight;
uniform float u_fogDensity;
uniform vec3 u_cameraPos;



uniform float u_moltenMapOffset;
uniform float u_rockDetailBumpStrength;
uniform float u_rockDetailDiffuseStrength;



uniform vec3 u_moltenColor;
uniform float u_moltenAlphaScalar;
uniform float u_moltenAlphaPower;

uniform vec3 u_dirtColor;
uniform vec3 u_vegColor;
uniform float u_vegBump;

uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_normalModelViewMatrix;
uniform float u_mapHeightOffset;

uniform float u_phaseA;
uniform float u_phaseB;
uniform float u_alphaA;
uniform float u_alphaB;

// Textures
uniform sampler2D s_heightData;
uniform sampler2D s_velocityData;
uniform sampler2D s_miscData;
uniform sampler2D s_normalData;
uniform sampler2D s_diffuseMap;
uniform sampler2D s_smudgeData;
uniform sampler2D s_moltenMapData;
uniform sampler2D s_dirtDiffuse;
uniform sampler2D s_rockDiffuse;

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
	vec4 out_viewPosition;
	vec4 out_forward;
	vec2 out_uv;
	float out_dirtAlpha;
	vec3 out_moltenColor;
	float out_moltenAlpha;
	vec3 out_rockNormal;
	float out_occlusion;
	float out_heat;
};

/*
out Varying
{
	vec4 out_viewPosition;
	vec2 out_uv;
	vec3 out_normal;
	float out_dirtAlpha;
	float out_vegAlpha;
	float out_moltenAlpha;
}
*/

////////////////////////////////////////////////////////////////
// STD Lib Functions
////////////////////////////////////////////////////////////////
vec4 sampleCombinedMip( sampler2D _sampler, vec2 _uv, int _minMip, int _maxMip, float _downSampleScalar );
vec3 reconstructNormal( vec2 normal2 );
float lightingGGX( vec3 N, vec3 V, vec3 L, float roughness, float F0 );

////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////
void main(void)
{
	// Pluck some values out of the texture data
	vec4 heightDataC = texture(s_heightData, in_uv);
	vec4 velocityDataC = texture(s_velocityData, in_uv);
	vec4 miscDataC = textureLod(s_miscData, in_uv, 1);
	vec4 smudgeDataC = texture(s_smudgeData, in_uv);
	vec4 diffuseData = texture(s_diffuseMap, in_uv);
	
	vec2 rockNormalPacked = sampleCombinedMip(s_normalData, in_uv, 0, 1, 0.5).zw;
	rockNormalPacked /= vec2(1.0 + 0.5 + 0.25);
	vec3 rockNormal = normalize( reconstructNormal(rockNormalPacked) );

	// Common values
	float rockHeight = heightDataC.x;
	float moltenHeight = heightDataC.y;
	float dirtHeight = heightDataC.z;
	float waterHeight = heightDataC.w;
	float vegAmount = smudgeDataC.w;
	float heat = miscDataC.x;
	float occlusion = 1.0f - miscDataC.w;
	//float moltenMapValue = clamp(miscDataC.y, 0.0, 1.0);
	vec3 viewDir = normalize(u_cameraPos);
	float steamStrength = smudgeDataC.z;
	float moltenMapValue = texture(s_moltenMapData, in_uv).x;

	vec4 position = vec4(in_position, 1.0f);
	position.y += rockHeight;
	position.y += moltenHeight;
	//position.y += dirtHeight;
	position.y += moltenMapValue * u_mapHeightOffset;
	//position.y += vegAmount * u_vegBump;

	vec4 viewPosition = u_modelViewMatrix * position;

	

	// Dirt
	/*
	vec3 dirtDiffuse = pow(u_dirtColor, vec3(2.2));// * mix(0.0, 1.0, diffuseData.z);
	dirtDiffuse *= pow( texture( s_dirtDiffuse, in_uv ).rgb, vec3(2.2) );

	float dirtAlpha = clamp((dirtHeight / 0.003), 0.0, 1.0);
	diffuse = mix(diffuse, dirtDiffuse, dirtAlpha);
	
	rockNormal += rockDetailBump * u_rockDetailBumpStrength * (1.0-dirtAlpha);
	rockNormal = normalize(rockNormal);

	// Vegetation
	vec3 vegDiffuse = pow(u_vegColor, vec3(2.2)) * mix(0.0, 1.0, diffuseData.z);
	float vegAlpha = clamp( vegAmount / 1.0, 0.0, 1.0 );
	diffuse = mix(diffuse, vegDiffuse, vegAlpha);
	roughness = mix( roughness, 0.7, vegAlpha );
	fresnel = mix( fresnel, 0.9, vegAlpha );
	*/

	
	
	// Molten
	float moltenAlpha = max( max(heat-0.3, 0.0) * u_moltenAlphaScalar, 0.0 );
	vec3 moltenColor = pow( mix( u_moltenColor, u_moltenColor * 1.25, moltenAlpha ), vec3(2.2) );
	
	// Molten flow map detail
	//vec2 moltenVelocity = velocityDataC.xy;
	//float moltenDiffuseDetailA = pow( texture(s_rockDiffuse, in_uv - moltenVelocity * u_phaseA * 0.6 + vec2(0.0)).b, 2.2 );
	//float moltenDiffuseDetailB = pow( texture(s_rockDiffuse, in_uv - moltenVelocity * u_phaseB * 0.6 + vec2(0.5)).b, 2.2 );
	//float moltenDiffuseDetail = max( moltenDiffuseDetailA * u_alphaA, moltenDiffuseDetailB * u_alphaB);
	//moltenColor -= moltenDiffuseDetail;

	// Bing it all together
	//vec3 outColor = (diffuse * (directLight + ambientlight));
	//outColor *= pow( clamp(1.0 - moltenAlpha, 0.0, 1.0), 4.0 );
	//outColor = mix( outColor, moltenColor, moltenAlpha );

	//outColor = mix( outColor, vec3(0.0,1.0,0.0), pow(steamStrength, 2.2) );
	
	// Output
	{
		out_viewPosition = viewPosition;
		out_forward = vec4(0.0);//vec4(outColor, 1.0f);//vec4(directLight);//vec4(moltenMapValue);//vec4(outColor, 1.0f);
		out_uv = in_uv;
		out_dirtAlpha = 1.0;
		out_moltenColor = moltenColor;
		out_moltenAlpha = moltenAlpha;
		out_rockNormal = rockNormal;
		out_occlusion = occlusion;
		out_heat = heat;
		gl_Position = u_mvpMatrix * position;
	}
} 