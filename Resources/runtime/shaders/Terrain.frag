#version 430 core

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
	float in_heat;
	vec3 in_rockNormal;
	float in_occlusion;
	float in_shadowing;
	vec2 in_scaledUV;
	vec4 in_heightData;
};

// Uniforms
uniform vec2 u_mouseScreenPos;
uniform ivec2 u_windowSize;

uniform float u_rockReflectivity;

uniform float u_hotRockRoughness;
uniform vec3 u_moltenColor;
uniform float u_moltenColorScalar;
uniform vec3 u_dirtColor;

uniform float u_glowScalar;
uniform float u_glowMipLevel = 4;
uniform float u_glowDistance = 0.1;

uniform vec3 u_cameraPos;
uniform float u_rockDetailBumpStrength;
uniform float u_rockDetailBumpSlopePower;

uniform vec3 u_lightDir;
uniform float u_lightDistance;
uniform float u_lightIntensity;
uniform float u_ambientLightIntensity;

uniform float u_bearingCreaseScalar;
uniform float u_lateralCreaseScalar;
uniform float u_creaseRatio;
uniform float u_creaseMipLevel;
uniform float u_creaseForwardScalar;
uniform float u_creaseMapRepeat;
uniform float u_creaseGridRepeat;

uniform float u_phaseA;
uniform float u_phaseB;
uniform float u_phaseAlpha;
uniform float u_flowOffset;

// Textures
uniform sampler2D s_smudgeData;
uniform sampler2D s_miscData;
uniform sampler2D s_heightData;
uniform sampler2D s_uvOffsetData;

uniform sampler2D s_lavaAlbedo;
uniform sampler2D s_lavaNormal;
uniform sampler2D s_lavaMaterial;

uniform sampler2D s_lavaLongAlbedo;
uniform sampler2D s_lavaLongNormal;
uniform sampler2D s_lavaLongMaterial;

uniform sampler2D s_lavaLatAlbedo;
uniform sampler2D s_lavaLatNormal;
uniform sampler2D s_lavaLatMaterial;

uniform samplerCube s_envMap;
uniform samplerCube s_irrMap;

uniform sampler2D s_moltenGradient;

uniform float u_textureBias = -0.0;

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
	return mix( _valueA, _valueB, _alpha );
}

vec3 heightMix( vec3 _valueA, vec3 _valueB, float _alpha, float _heightA, float _heightB )
{
	return mix( _valueA, _valueB, _alpha );
}

vec4 bilinearMix( vec4 _valueTL, vec4 _valueTR, vec4 _valueBL, vec4 _valueBR, vec2 _ratio )
{
	vec4 ret = _ratio.x * _ratio.y * _valueBR;
	ret += (1.0-_ratio.x) * _ratio.y * _valueBL;
	ret += _ratio.x * (1.0-_ratio.y) * _valueTR;
	ret += (1.0-_ratio.x) * (1.0-_ratio.y) * _valueTL;

	return ret;
}

vec3 bilinearMix( vec3 _valueTL, vec3 _valueTR, vec3 _valueBL, vec3 _valueBR, vec2 _ratio )
{
	vec3 ret = _ratio.x * _ratio.y * _valueBR;
	ret += (1.0-_ratio.x) * _ratio.y * _valueBL;
	ret += _ratio.x * (1.0-_ratio.y) * _valueTR;
	ret += (1.0-_ratio.x) * (1.0-_ratio.y) * _valueTL;

	return ret;
}

vec4 samplePhasedMap( sampler2D _sampler, sampler2D _heightSampler, vec2 _uv, vec4 _uvOffset, float _angle )
{
	_uvOffset.xy = rotateBy( _uvOffset.xy, _angle );
	_uvOffset.zw = rotateBy( _uvOffset.zw, _angle );

	vec2 uvA = (_uv * u_creaseMapRepeat) - _uvOffset.xy * u_flowOffset;
	vec2 uvB = (_uv * u_creaseMapRepeat) - _uvOffset.zw * u_flowOffset;

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

	vec2 uvA = (_uv * u_creaseMapRepeat) - _uvOffset.xy * u_flowOffset;
	vec2 uvB = (_uv * u_creaseMapRepeat) - _uvOffset.zw * u_flowOffset;

	vec3 sampleA = decodeNormalDXT( texture( _sampler, uvA, u_textureBias ) );
	vec3 sampleB = decodeNormalDXT( texture( _sampler, uvB, u_textureBias ) );

	float heightSampleA = texture( _heightSampler, uvA, u_textureBias ).y;
	float heightSampleB = texture( _heightSampler, uvB, u_textureBias ).y;

	return normalize( heightMix( sampleA, sampleB, u_phaseAlpha, heightSampleA, heightSampleB ) );
}

vec4 sampleBilinearPhasedMap( sampler2D _sampler, sampler2D _heightSampler, vec2 _uvTL, vec2 _uvTR, vec2 _uvBL, vec2 _uvBR, vec4 _uvOffset, float _angle, vec2 _ratio )
{
	vec4 sampleTL = samplePhasedMap( _sampler, _heightSampler, _uvTL, _uvOffset, _angle );
	vec4 sampleTR = samplePhasedMap( _sampler, _heightSampler, _uvTR, _uvOffset, _angle );
	vec4 sampleBL = samplePhasedMap( _sampler, _heightSampler, _uvBL, _uvOffset, _angle );
	vec4 sampleBR = samplePhasedMap( _sampler, _heightSampler, _uvBR, _uvOffset, _angle );

	return bilinearMix(sampleTL, sampleTR, sampleBL, sampleBR, _ratio);
}

vec3 sampleBilinearPhasedMapNormalDXT( sampler2D _sampler, sampler2D _heightSampler, vec2 _uvTL, vec2 _uvTR, vec2 _uvBL, vec2 _uvBR, vec4 _uvOffset, float _angle, vec2 _ratio )
{
	vec3 sampleTL = samplePhasedMapNormalDXT( _sampler, _heightSampler, _uvTL, _uvOffset, _angle );
	vec3 sampleTR = samplePhasedMapNormalDXT( _sampler, _heightSampler, _uvTR, _uvOffset, _angle );
	vec3 sampleBL = samplePhasedMapNormalDXT( _sampler, _heightSampler, _uvBL, _uvOffset, _angle );
	vec3 sampleBR = samplePhasedMapNormalDXT( _sampler, _heightSampler, _uvBR, _uvOffset, _angle );

	return normalize( bilinearMix(sampleTL, sampleTR, sampleBL, sampleBR, _ratio) );
}


void main(void)
{
	UpdateMousePosition();

	// Common values
	vec3 viewDir = normalize(u_cameraPos-in_worldPosition);
	vec3 lightDir = normalize(u_lightDir * u_lightDistance - in_worldPosition);
	vec4 uvOffsetSample = texture( s_uvOffsetData, in_uv );

	vec3 sampledAlbedo;
	vec3 sampledNormal;
	vec4 sampledMaterial;
	float creaseAmount = 1.0;
	{
		vec2 flow = textureLod( s_smudgeData, in_uv, u_creaseMipLevel ).xy;
		vec2 forwardFlow = textureLod( s_smudgeData, in_uv + normalize(flow) * u_creaseForwardScalar, u_creaseMipLevel ).xy;

		float projectionLength = dot( forwardFlow, flow / length(flow) );
		projectionLength -= length(flow) * u_creaseRatio;
		float stretchRatio = clamp( max(0, projectionLength) * u_bearingCreaseScalar, 0.0, 1.0 );
		float compressionRatio = clamp( max(0, -projectionLength) * u_lateralCreaseScalar, 0.0, 1.0 );
		
		float gridSize = 1.0 / u_creaseGridRepeat;
		ivec2 gridCell = ivec2( in_uv * u_creaseGridRepeat );
		vec2 gridCellPos = in_uv * u_creaseGridRepeat;

		vec2 gridCellCenterTL = (gridCell * gridSize);
		vec2 gridCellCenterTR = ((gridCell + ivec2(1,0)) * gridSize);
		vec2 gridCellCenterBL = ((gridCell + ivec2(0,1)) * gridSize);
		vec2 gridCellCenterBR = ((gridCell + ivec2(1,1)) * gridSize);
		vec2 ratio = (gridCellPos - gridCell);

		float angle = -atan(flow.y, flow.x);
		angle = isnan(angle) ? 0.0 : angle;

		vec2 uvTL = rotateAroundBy( in_uv, angle, gridCellCenterTL);
		vec2 uvTR = rotateAroundBy( in_uv, angle, gridCellCenterTR);
		vec2 uvBL = rotateAroundBy( in_uv, angle, gridCellCenterBL);
		vec2 uvBR = rotateAroundBy( in_uv, angle, gridCellCenterBR);

		vec4 sampledMaterialStill = samplePhasedMap( s_lavaMaterial, s_lavaMaterial, in_scaledUV, uvOffsetSample, 0.0 ).rgba;
		vec4 sampledMaterialLong = sampleBilinearPhasedMap( s_lavaLongMaterial, s_lavaLongMaterial, uvTL, uvTR, uvBL, uvBR, uvOffsetSample, angle, ratio ).rgba;
		vec4 sampledMaterialLat = sampleBilinearPhasedMap( s_lavaLatMaterial, s_lavaLatMaterial, uvTL, uvTR, uvBL, uvBR, uvOffsetSample, angle, ratio ).rgba;
		sampledMaterial = sampledMaterialStill;//mix( sampledMaterialStill, sampledMaterialLong, compressionRatio );
		//sampledMaterial = mix( sampledMaterial, sampledMaterialLat, stretchRatio );
		
		vec3 sampledAlbedoStill = samplePhasedMap( s_lavaAlbedo, s_lavaMaterial, in_scaledUV, uvOffsetSample, 0.0 ).rgb;
		vec3 sampledAlbedoLong = sampleBilinearPhasedMap( s_lavaLongAlbedo, s_lavaLongMaterial, uvTL, uvTR, uvBL, uvBR, uvOffsetSample, angle, ratio ).rgb;
		vec3 sampledAlbedoLat = sampleBilinearPhasedMap( s_lavaLatAlbedo, s_lavaLatMaterial, uvTL, uvTR, uvBL, uvBR, uvOffsetSample, angle, ratio ).rgb;
		sampledAlbedo = sampledAlbedoStill;//mix( sampledAlbedoStill, sampledAlbedoLong, compressionRatio );
		//sampledAlbedo = mix( sampledAlbedo, sampledAlbedoLat, stretchRatio );
		sampledAlbedo = pow(sampledAlbedo, vec3(2.2));
		
		vec3 sampledNormalStill = samplePhasedMapNormalDXT( s_lavaNormal, s_lavaMaterial, in_scaledUV, uvOffsetSample, 0.0 );

		vec3 sampledNormalLong = sampleBilinearPhasedMapNormalDXT( s_lavaLongNormal, s_lavaLongMaterial, uvTL, uvTR, uvBL, uvBR, uvOffsetSample, angle, ratio ).xyz;
		sampledNormalLong = rotateZ( sampledNormalLong, angle );

		vec3 sampledNormalLat = sampleBilinearPhasedMapNormalDXT( s_lavaLatNormal, s_lavaLatMaterial, uvTL, uvTR, uvBL, uvBR, uvOffsetSample, angle, ratio ).xyz;
		sampledNormalLat = rotateZ( sampledNormalLong, angle );

		sampledNormal = sampledNormalStill;//mix( sampledNormalStill, sampledNormalLong, compressionRatio );
		//sampledNormal = mix( sampledNormal, sampledNormalLat, stretchRatio );

		sampledNormal = normalize(sampledNormal);
	}

	// Rock normal
	vec3 rockNormal = in_rockNormal;
	rockNormal = rotateX( rockNormal, sampledNormal.y * u_rockDetailBumpStrength ); 
	rockNormal = rotateZ( rockNormal, -sampledNormal.x * u_rockDetailBumpStrength ); 
	rockNormal = normalize(rockNormal);

	// Direct light
	float roughness = sampledMaterial.r;
	float textureAO = sampledMaterial.g;

	vec3 specularColor = vec3(pow(u_rockReflectivity, 2.2));

	float moltenRatio = 1.0 - ( min( in_heat * 10.0, 1.0 ) );
	specularColor *= moltenRatio;
	sampledAlbedo *= moltenRatio;

	// Direct light
	//vec3 lightColor = vec3(1.0,1.0,1.0);
	vec3 directLight = vec3(0.0);//pointLightContribution( rockNormal, lightDir, viewDir, sampledAlbedo, specularColor, roughness, lightColor, u_lightIntensity ) * (1.0-in_shadowing);

	// Ambient light
	vec3 ambientLight = IBLContribution( rockNormal, viewDir, sampledAlbedo, specularColor, roughness, s_envMap, s_irrMap, u_ambientLightIntensity, in_occlusion * textureAO);
	
	// Local glow from heat
	vec3 heatLight = vec3(0.0);
	{
		vec3 sampleOffset = normalize( vec3( rockNormal.x, 0.0, rockNormal.z ) );
		
		vec3 samplePos = in_worldPosition + sampleOffset * u_glowDistance;
		vec2 sampleUV = samplePos.xz + vec2(0.5);
		vec4 sampleHeightData = texture(s_heightData, sampleUV);
		samplePos.y = sampleHeightData.x + sampleHeightData.y + sampleHeightData.z;
		samplePos.y *= 2.0;

		float sampleHeat = textureLod( s_miscData, sampleUV, u_glowMipLevel ).x;
		vec3 sampleDir = samplePos - in_worldPosition;
		float sampleDis = length(sampleDir);
		sampleDir = normalize(sampleDir);

		vec3 heatColor = pow( texture(s_moltenGradient, vec2(sampleHeat * 0.5, 0.5)).rgb, vec3(2.2) );
		vec3 sampleHeatLight = pointLightContribution( rockNormal, sampleDir, viewDir, sampledAlbedo, specularColor, roughness, heatColor, u_glowScalar);
		sampleHeatLight /= (1.0 + sampleDis*sampleDis);

		heatLight += sampleHeatLight;
	}

	// Bring it all together
	vec3 outColor = directLight + ambientLight + heatLight * textureAO;

	// Add emissve elements
	float moltenMap = sampledMaterial.b;
	float heat = pow(clamp(in_heat, 0.0, 1.0), 0.5);
	float moltenAlphaA = pow( moltenMap, mix( 2.0, 0.4, heat ) ) * heat;
	float moltenAlphaB = pow( moltenMap, 4.0 ) * (1.0 - heat) * heat * 4;
	float moltenAlpha = clamp( moltenAlphaA + moltenAlphaB, 0.0, 1.0 );
	

	vec3 moltenColor = pow( texture(s_moltenGradient, vec2(moltenAlpha, 0.5)).rgb, vec3(2.2) );
	moltenColor *= 1.0 + max(in_heat-1.0, 0.0);
	outColor += moltenColor;

	out_worldNormal = vec4(rockNormal, 0.0);
	
	out_viewPosition = in_viewPosition;

	//.x = in_heightData.x * 10.0;
	//outColor.y = in_heightData.y * 10.0;
	//outColor.z = 0.0;

	out_forward = vec4( outColor, 1.0 );
}
