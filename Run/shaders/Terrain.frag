#version 430 core

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
	float in_dirtAlpha;
	vec3 in_rockNormal;
	float in_occlusion;
	float in_shadowing;
	vec2 in_scaledUV;
	vec4 in_heightData;
	vec4 in_smudgeData;
	vec3 in_albedoFluidColor;
	vec4 in_miscData;
	vec4 in_derivedData;
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
uniform sampler2D s_uvOffsetData;

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
float packUnorm4x8f( vec4 );
vec4 unpackUnorm4x8f( float );

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////
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

vec2 rotateBy( vec2 _pt, float _angle )
{
	float cosValue = cos(_angle);
	float sinValue = sin(_angle);
	return vec2( _pt.x * cosValue - _pt.y * sinValue, _pt.x * sinValue + _pt.y * cosValue );
}

vec2 rotateAroundBy( vec2 _pt, float _angle, vec2 _offset )
{
	_pt -= _offset;
	_pt = rotateBy( _pt, _angle );
	_pt += _offset;

	return _pt;
}

vec4 heightMix( vec4 _valueA, vec4 _valueB, float _alpha, float _heightA, float _heightB )
{
	return mix( _valueB, _valueA, smoothstep( -u_heightBlendWidth, u_heightBlendWidth, (1.0-_heightA) - _alpha ) );
}

vec3 heightMix( vec3 _valueA, vec3 _valueB, float _alpha, float _heightA, float _heightB )
{
	return mix( _valueB, _valueA, smoothstep( -u_heightBlendWidth, u_heightBlendWidth, (1.0-_heightA) - _alpha ) );
}

vec4 samplePhasedMap( sampler2D _sampler, sampler2D _heightSampler, vec2 _uv, vec4 _uvOffset, float _angle )
{
	_uvOffset.xy = rotateBy( _uvOffset.xy, _angle );
	_uvOffset.zw = rotateBy( _uvOffset.zw, _angle );

	vec2 uvA = (_uv ) - _uvOffset.xy * u_flowOffset;
	vec2 uvB = (_uv ) - _uvOffset.zw * u_flowOffset;

	vec4 sampleA = texture( _sampler, uvA, u_textureBias );
	vec4 sampleB = texture( _sampler, uvB, u_textureBias );

	float heightSampleA = texture( _heightSampler, uvA, u_textureBias ).y;
	float heightSampleB = texture( _heightSampler, uvB, u_textureBias ).y;

	return heightMix( sampleA, sampleB, u_phaseAlpha, heightSampleA, heightSampleB );
}

vec3 samplePhasedMapNormalDXT( sampler2D _sampler, sampler2D _heightSampler, vec2 _uv, vec4 _uvOffset, float _angle )
{
	_uvOffset.xy = rotateBy( _uvOffset.xy, _angle );
	_uvOffset.zw = rotateBy( _uvOffset.zw, _angle );

	vec2 uvA = (_uv ) - _uvOffset.xy * u_flowOffset;
	vec2 uvB = (_uv ) - _uvOffset.zw * u_flowOffset;

	vec3 sampleA = decodeNormalDXT( texture( _sampler, uvA, u_textureBias ) );
	vec3 sampleB = decodeNormalDXT( texture( _sampler, uvB, u_textureBias ) );

	float heightSampleA = texture( _heightSampler, uvA, u_textureBias ).y;
	float heightSampleB = texture( _heightSampler, uvB, u_textureBias ).y;

	return normalize( heightMix( sampleA, sampleB, u_phaseAlpha, heightSampleA, heightSampleB ) );
}

float getCreaseValue( vec2 _uv )
{
	vec4 smudgeData = texture(s_smudgeData, _uv);

	vec2 ray = vec2(smudgeData.y, -smudgeData.x);
	//vec2 ray = vec2(smudgeData.x, smudgeData.y);
	float rayLength = length(ray);
	ray /= rayLength;

	vec2 E = _uv;
	vec2 L = _uv + ray * 40;
	vec2 C = vec2(0.5);
	float r = 20;
	vec2 d = L-E;
	vec2 f = E-C;

	float a = dot(d,d);
	float b = 2.0 * dot(f, d);
	float c = dot(f, f) - r*r;
	float discriminant = sqrt( b*b-4*a*c );
	float t = (-b + discriminant) / (2.0*a);
	float delta = length( t * d );

	float value = (sin(delta*u_creaseFrequency) + 1.0) * 0.5;
	value = pow( abs(value), 0.4 );
	//float value = fract(delta*u_creaseFrequency);

	if ( isnan(value) )
		return 0.0;

	return value * rayLength;
}

vec3 getCreaseTangent( vec2 _uv, float _width )
{
	float creaseValueL = getCreaseValue( _uv - vec2( _width, 0.0 ) );
	float creaseValueR = getCreaseValue( _uv + vec2( _width, 0.0 ) );
	float creaseValueT = getCreaseValue( _uv - vec2( 0.0, _width ) );
	float creaseValueB = getCreaseValue( _uv + vec2( 0.0, _width ) );

	vec3 normal = vec3(0.0,0.0,100.0);

	normal.x = creaseValueR - creaseValueL;
	normal.y = creaseValueT - creaseValueB;

	normal = normalize(normal);

	return normal;
}

void main(void)
{
	UpdateMousePosition();

	// Common values
	vec3 viewDir = normalize(u_cameraPos-in_worldPosition);
	vec3 lightDir = normalize(u_lightDir * u_lightDistance - in_worldPosition);
	vec4 uvOffsetSample = texture( s_uvOffsetData, in_uv );
	vec4 smudgeDataC = texture(s_smudgeData, in_uv);
	
	// Rock material
	vec3 rockAlbedo = samplePhasedMap( s_lavaAlbedo, s_lavaMaterial, in_scaledUV, uvOffsetSample, 0.0 ).rgb;
		
	vec3 rockNormal = in_rockNormal;
	vec3 rockNormalTangent = samplePhasedMapNormalDXT( s_lavaNormal, s_lavaMaterial, in_scaledUV, uvOffsetSample, 0.0 );
	rockNormal = rotateX( rockNormal, rockNormalTangent.y * u_rockNormalStrength ); 
	rockNormal = rotateZ( rockNormal, -rockNormalTangent.x * u_rockNormalStrength ); 

	// Smudge
	float creaseValue = min( getCreaseValue(in_uv + rockNormalTangent.xy * u_creaseDistortStrength ), 1.0 );
	vec3 creaseTangent = getCreaseTangent(in_uv + rockNormalTangent.xy * u_creaseDistortStrength, 0.005);

	rockAlbedo = mix( rockAlbedo, in_albedoFluidColor, 0.7 );
	
	rockNormal = rotateX( rockNormal, creaseTangent.y * u_creaseNormalStrength ); 
	rockNormal = rotateZ( rockNormal, -creaseTangent.x * u_creaseNormalStrength ); 
	rockNormal = normalize(rockNormal);

	vec4 rockMaterialParams = samplePhasedMap( s_lavaMaterial, s_lavaMaterial, in_scaledUV, uvOffsetSample, 0.0 ).rgba;
	rockMaterialParams.r = 0.4;
	vec3 rockSpecularColor = degamma( vec3(u_rockReflectivity) );

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
	float textureAO = mix( 1.0, materialParams.g, 0.5 ) * mix( 1.0, creaseValue, 0.6 );

	// Make albedo/specular darker when hot
	float moltenRatio = 1.0 - ( min( in_miscData.x * 2.0, 1.0 ) );
	specularColor *= moltenRatio;
	albedo *= moltenRatio;

	albedo *= vec3( sin( in_miscData.y * PI * 2.0 ) * 0.5 + 1.0 );
	
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
	float heat = pow(clamp(in_miscData.x, 0.0, 1.0), 0.5);
	float moltenAlphaA = pow( moltenMap, mix( 1.5, 0.4, heat ) ) * heat;
	float moltenAlphaB = pow( moltenMap, 2.5 ) * (1.0 - heat) * heat * 6;
	float moltenAlpha = clamp( moltenAlphaA + moltenAlphaB, 0.0, 1.0 );
	
	vec3 moltenColor = degamma( texture(s_moltenGradient, vec2(moltenAlpha * 0.96, 0.5)).rgb );
	moltenColor *= 1.0 + max(in_miscData.x, 0.0);
	outColor += moltenColor;
	out_worldNormal = vec4(normal, 0.0);
	out_viewPosition = in_viewPosition;
	out_forward = vec4( outColor, 1.0 );
	
	vec2 velocitySample = texture( s_fluidVelocityData, in_uv ).xy;
	
	velocitySample *= 1.0;
	velocitySample += 0.5;
	
	float divergenceSample = texture( s_divergenceData, in_uv ).r;
	divergenceSample *= 1.0;
	divergenceSample += 0.5;
	
	float pressureSample = texture( s_pressureData, in_uv ).g;
	pressureSample *= 1000.0;
	pressureSample += 0.5;
	
	//out_forward = pow( vec4( velocitySample, pressureSample, 0.0 ), vec4(2.2));//densitySample.x, 0.0 );
	
}
