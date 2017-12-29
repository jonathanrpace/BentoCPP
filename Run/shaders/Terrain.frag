#version 440 core

const float PI = 3.14159265359;

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Varying
in Varying
{
	vec3 in_worldPosition;
	vec4 in_viewPosition;
	vec2 in_uv;
	vec4 in_moltenUVOffsets;
	float in_dirtAlpha;
	vec3 in_rockNormal;
	float in_occlusion;
	float in_shadowing;
	vec2 in_scaledUV;
	vec4 in_heightData;
	vec4 in_smudgeData;
	vec3 in_albedoFluidColor;
	vec4 in_miscData;
};

// Uniforms
uniform vec2 u_mouseScreenPos;
uniform ivec2 u_windowSize;
uniform vec3 u_cameraPos;

// Lighting
uniform vec3 u_lightDir;
uniform float u_lightDistance;
uniform float u_lightIntensity;
uniform float u_ambientLightIntensity;

// Motion / Flow
uniform float u_phaseA;
uniform float u_phaseB;
uniform float u_phaseAlpha;
uniform float u_flowOffset;

// Rock
uniform float u_rockReflectivity;
uniform float u_rockNormalStrength;
uniform float u_rockDetailBumpSlopePower;

// Molten
uniform vec3 u_moltenColor;
uniform float u_moltenColorScalar;
uniform float u_heightBlendWidth = 0.2;

// Creases
uniform float u_creaseFrequency;
uniform float u_creaseNormalStrength;
uniform float u_creaseDistortStrength;

// Dirt
uniform vec3 u_dirtColor;

// Glow
uniform float u_glowScalar;
uniform float u_glowMipLevel = 4;
uniform float u_glowDistance = 0.1;

// Textures
uniform sampler2D s_smudgeData;
uniform sampler2D s_miscData;
uniform sampler2D s_heightData;

uniform sampler2D s_fluidVelocityData;
uniform sampler2D s_divergenceData;
uniform sampler2D s_pressureData;

uniform sampler2D s_lavaAlbedo;
uniform sampler2D s_lavaNormal;
uniform sampler2D s_lavaMaterial;

uniform samplerCube s_envMap;
uniform samplerCube s_irrMap;

uniform sampler2D s_moltenGradient;

uniform float u_textureBias = -1.0;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_viewPosition;
layout( location = 1 ) out vec4 out_worldNormal;
layout( location = 2 ) out vec4 out_forward;

////////////////////////////////////////////////////////////////
// Read/Write buffers
////////////////////////////////////////////////////////////////

layout( std430, binding = 0 ) buffer MousePositionBuffer
{
	int mouseBufferZ;
	int mouseBufferU;
	int mouseBufferV;
};

////////////////////////////////////////////////////////////////
// STD Lib Functions
////////////////////////////////////////////////////////////////
vec3 pointLightContribution(vec3 N,	vec3 L,	vec3 V,	vec3 diffColor,	vec3 specColor,	float roughness, vec3 lightColor, float lightIntensity );
vec3 IBLContribution(vec3 N, vec3 V, vec3 diffColor, vec3 specColor, float roughness, samplerCube envMap, samplerCube irrMap, float lightIntensity, float ambientOcclusion);

vec3 decodeNormalDXT( vec4 _sample );

vec3 degamma( vec3 );

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

// Project the surface gradient (dhdx, dhdy) onto the surface (n, dpdx, dpdy)
vec3 CalculateSurfaceGradient(vec3 n, vec3 dpdx, vec3 dpdy, float dhdx, float dhdy)
{
    vec3 r1 = cross(dpdy, n);
    vec3 r2 = cross(n, dpdx);
 
    return (r1 * dhdx + r2 * dhdy) / dot(dpdx, r1);
}
 
// Move the normal away from the surface normal in the opposite surface gradient direction
vec3 PerturbNormal(vec3 n, vec3 dpdx, vec3 dpdy, float dhdx, float dhdy)
{
    return normalize(n - CalculateSurfaceGradient(n, dpdx, dpdy, dhdx, dhdy));
}

// Calculate the surface normal using screen-space partial derivatives of the height field
vec3 CalculateSurfaceNormal(vec3 position, vec3 normal, float height)
{
    vec3 dpdx = dFdx(position);
    vec3 dpdy = dFdy(position);
 
    float dhdx = dFdx(height);
    float dhdy = dFdy(height);
 
    return PerturbNormal(normal, dpdx, dpdy, dhdx, dhdy);
}

void UpdateMousePosition()
{
	int fragViewZ = int(-in_viewPosition.z * 256.0f);

	vec4 screenPos = gl_FragCoord;
	screenPos.xy /= u_windowSize;
	screenPos.xy *= 2.0f;
	screenPos.xy -= 1.0f;

	vec2 maxDis = 1.0f / u_windowSize;
	maxDis *= 4.0f;

	vec2 dis = abs(u_mouseScreenPos-screenPos.xy);

	if ( dis.x > maxDis.x || dis.y > maxDis.y )
		return;

	int currViewZ = atomicMin(mouseBufferZ, fragViewZ);

	if ( fragViewZ == currViewZ )
	{
		ivec2 fragUV = ivec2(in_uv * 255.0f);
		atomicExchange(mouseBufferU, fragUV.x);
		atomicExchange(mouseBufferV, fragUV.y);
	}
}

vec3 rotateX( vec3 _dir, float _angle )
{
	float cosValue = cos(_angle);
	float sinValue = sin(_angle);
	return vec3
	( 
		_dir.x, 
		_dir.y * cosValue - _dir.z * sinValue, 
		_dir.y * sinValue + _dir.z * cosValue
	);
}

vec3 rotateZ( vec3 _dir, float _angle )
{
	float cosValue = cos(_angle);
	float sinValue = sin(_angle);
	return vec3
	( 
		_dir.x * cosValue - _dir.y * sinValue, 
		_dir.x * sinValue + _dir.y * cosValue,
		_dir.z
	);
}

vec4 heightMix( vec4 _valueA, vec4 _valueB, float _alpha, float _heightA, float _heightB )
{
	return mix( _valueB, _valueA, smoothstep( -u_heightBlendWidth, u_heightBlendWidth, (1.0-_heightA) - _alpha ) );
}

vec3 heightMix( vec3 _valueA, vec3 _valueB, float _alpha, float _heightA, float _heightB )
{
	return mix( _valueB, _valueA, smoothstep( -u_heightBlendWidth, u_heightBlendWidth, (1.0-_heightA) - _alpha ) );
}

vec4 sampleSplatMap( sampler2D _sampler, sampler2D _heightSampler, vec2 _uv, vec2 _uvOffset )
{
	return vec4(0.0);
}

vec4 samplePhasedMap( sampler2D _sampler, sampler2D _heightSampler, vec2 _uv, vec4 _uvOffset )
{
	vec2 uvA = (_uv ) - _uvOffset.xy * u_flowOffset;
	vec2 uvB = (_uv ) - _uvOffset.zw * u_flowOffset;

	vec4 sampleA = texture( _sampler, uvA, u_textureBias );
	vec4 sampleB = texture( _sampler, uvB, u_textureBias );

	float heightSampleA = texture( _heightSampler, uvA, u_textureBias ).a;
	float heightSampleB = texture( _heightSampler, uvB, u_textureBias ).a;

	return heightMix( sampleA, sampleB, u_phaseAlpha, heightSampleA, heightSampleB );
}

void main(void)
{
	UpdateMousePosition();

	// Common values
	vec3 viewDir = normalize(u_cameraPos-in_worldPosition);
	vec3 lightDir = normalize(u_lightDir * u_lightDistance - in_worldPosition);
	
	vec4 smudgeDataC = texture(s_smudgeData, in_uv);
	
	// Rock material
	vec3 rockAlbedo = samplePhasedMap( s_lavaAlbedo, s_lavaMaterial, in_scaledUV, in_moltenUVOffsets ).rgb;
	vec4 rockMaterialParams = samplePhasedMap( s_lavaMaterial, s_lavaMaterial, in_scaledUV, in_moltenUVOffsets ).rgba;
	vec3 rockSpecularColor = degamma( vec3(u_rockReflectivity) );

	vec3 rockNormal = in_rockNormal;
	float rockHeight = rockMaterialParams.a * u_rockNormalStrength * 0.01;
	rockNormal = CalculateSurfaceNormal( in_worldPosition, rockNormal, rockHeight );

	// Dirt material
	vec3 dirtAlbedo = u_dirtColor;															// TODO: Sample from dirt map
	vec3 dirtNormal = in_rockNormal;														// TODO: Sampler from dirt tangent map
	vec4 dirtMaterialParams = vec4( 0.6, 1.0, 0.0, 0.0 );									// TODO: Sample from map
	vec3 dirtSpecularColor = vec3(0.1);														// TODO: Add uniform

	// Blend rock/dirt
	float dirtBlendAlpha = in_dirtAlpha;													// TODO: Scramble with height/grunge/normal map
	//dirtBlendAlpha = mix( dirtBlendAlpha - (1.0-rockMaterialParams.b), 1.0, dirtBlendAlpha );
	dirtBlendAlpha -= pow( abs(rockNormal.x), 2.0 );
	dirtBlendAlpha -= pow( abs(rockNormal.z), 2.0 );
	dirtBlendAlpha = max(0.0, dirtBlendAlpha);

	vec3 albedo = mix( rockAlbedo, dirtAlbedo, dirtBlendAlpha );
	vec3 normal = normalize( mix( rockNormal, dirtNormal, dirtBlendAlpha ) );				// TOOD: Investigate better way to blend normals
	vec3 specularColor = mix( rockSpecularColor, dirtSpecularColor, dirtBlendAlpha );
	vec4 materialParams = mix( rockMaterialParams, dirtMaterialParams, dirtBlendAlpha );

	float roughness = materialParams.r;
	roughness *= 0.5;
	float textureAO = materialParams.g;//mix( 1.0, materialParams.g, 0.5 );// * mix( 1.0, creaseValue, 0.6 );

	// Make albedo/specular darker when hot
	float moltenRatio = 1.0 - ( min( in_miscData.x / 0.1, 1.0 ) );
	specularColor *= moltenRatio;
	//albedo *= moltenRatio;

	//albedo *= vec3( sin( in_miscData.y * PI * 2.0 ) * 0.5 + 1.0 );
	//albedo *= in_miscData.y;
	
	// Direct light
	//vec3 lightColor = vec3(1.0,1.0,1.0);
	vec3 directLight = vec3(0.0);//pointLightContribution( normal, lightDir, viewDir, albedo, specularColor, roughness, lightColor, u_lightIntensity ) * (1.0-in_shadowing);

	// Ambient light
	vec3 ambientLight = IBLContribution( normal, viewDir, albedo, specularColor, roughness, s_envMap, s_irrMap, u_ambientLightIntensity, in_occlusion * textureAO);
	
	// Local glow from heat
	vec3 heatLight = vec3(0.0);
	{
		vec3 sampleOffset = normalize( vec3( normal.x, 0.1, normal.z ) );
		
		vec3 samplePos = in_worldPosition + sampleOffset * u_glowDistance;
		vec2 sampleUV = samplePos.xz + vec2(0.5);
		vec4 sampleHeightData = texture(s_heightData, sampleUV);
		samplePos.y = sampleHeightData.x + sampleHeightData.y + sampleHeightData.z;
		samplePos.y += 0.1;

		float sampleHeat = textureLod( s_miscData, sampleUV, u_glowMipLevel ).x;
		vec3 sampleDir = samplePos - in_worldPosition;
		float sampleDis = length(sampleDir);
		sampleDir /= sampleDis;
		
		vec3 heatColor = degamma( texture(s_moltenGradient, vec2(sampleHeat * 0.5, 0.5)).rgb );
		vec3 sampleHeatLight = pointLightContribution( normal, sampleDir, viewDir, albedo, specularColor, roughness, heatColor, u_glowScalar);
		sampleHeatLight /= (1.0 + sampleDis*sampleDis);

		heatLight += sampleHeatLight;
	}

	heatLight = (isnan(heatLight.x) || isnan(heatLight.y) || isnan(heatLight.z)) ? vec3(0.0) : heatLight;

	// Bring it all together
	vec3 outColor = directLight + ambientLight + heatLight * textureAO;

	// Add emissve elements
	float moltenMap = rockMaterialParams.b;
	float heat = pow(clamp(in_miscData.x * 0.5, 0.0, 2.0), 0.2) * in_miscData.x * 0.1;
	heat = min(1.0, heat);
	float moltenAlphaA = pow( moltenMap, mix( 1.5, 0.4, heat ) ) * heat;
	float moltenAlphaB = pow( moltenMap, 2.5 ) * (1.0 - heat) * heat * 6;
	float moltenAlpha = clamp( moltenAlphaA + moltenAlphaB, 0.0, 1.0 );
	
	vec3 moltenColor = degamma( texture(s_moltenGradient, vec2(moltenAlpha * 0.96, 0.5)).rgb );
	moltenColor *= 1.0 + max(in_miscData.x, 0.0);
	
	/*
	float pressureSample = texture( s_pressureData, in_uv ).r;
	pressureSample *= 10000.0;
	pressureSample += 0.5;
	moltenColor *= 0.1;
	moltenColor.rgb += pressureSample;
	*/
	
	outColor += moltenColor;
	out_worldNormal = vec4(normal, 0.0);
	out_viewPosition = in_viewPosition;
	out_forward = vec4( outColor, 1.0 );

	/*
	vec2 velocitySample = texture( s_fluidVelocityData, in_uv ).xy;
	
	velocitySample *= 1.0;
	velocitySample += 0.5;
	
	float divergenceSample = texture( s_divergenceData, in_uv ).r;
	divergenceSample *= 1.0;
	divergenceSample += 0.5;
	
	
	
	//out_forward = vec4( pressureSample, pressureSample, pressureSample, 0.0 );
	//out_forward = pow( vec4( velocitySample, pressureSample, 0.0 ), vec4(2.2));//densitySample.x, 0.0 );
	*/
}
