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
	vec3 in_moltenColor;
	float in_moltenAlpha;
	vec3 in_rockNormal;
	float in_occlusion;
	float in_heat;
	float in_shadowing;
};

// Uniforms
uniform vec2 u_mouseScreenPos;
uniform ivec2 u_windowSize;

uniform float u_rockReflectivity;
uniform float u_rockFresnelA;
uniform float u_rockFresnelB;

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
uniform sampler2D s_velocityData;
uniform sampler2D s_miscData;
uniform sampler2D s_heightData;

uniform sampler2D s_lavaAlbedo;
uniform sampler2D s_lavaNormal;
uniform sampler2D s_lavaMaterial;

uniform sampler2D s_lavaLongAlbedo;
uniform sampler2D s_lavaLongNormal;
uniform sampler2D s_lavaLongMaterial;

uniform sampler2D s_lavaLatAlbedo;
uniform sampler2D s_lavaLatNormal;
uniform sampler2D s_lavaLatMaterial;

uniform float u_textureBias = -1.0;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_viewPosition;
layout( location = 1 ) out vec4 out_forward;

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
float lightingGGX( vec3 N, vec3 V, vec3 L, float roughness, float F0 );
vec3 lightingGGXAlbedo( vec3 N, vec3 V, vec3 L, float roughness, float F0, float reflectivity, vec3 albedo );
vec4 sampleCombinedMip( sampler2D _sampler, vec2 _uv, int _minMip, int _maxMip, float _downSampleScalar );
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

vec4 heightMix( vec4 _valueA, vec4 _valueB, float _alpha, float _heightA, float _heightB, float _blendWidth )
{
	//_heightA = max( 0.0, _heightA - _alpha );
	//_heightB = -(1.0-_alpha) + _heightB;

	_heightB *= _alpha;
	_heightA *= (1.0-_alpha);

	return _heightA > _heightB ? _valueA : _valueB;

	float diff = _heightB - _heightA;

	if ( diff >= 0.0 )
	{
		return mix( _valueA, _valueB, min(1.0, diff / _blendWidth) );
	}

	return mix( _valueB, _valueA, min(1.0, (-diff) / _blendWidth) );
	
	//float blendAlpha = diff * _blendWidth;
	//return mix( _valueA, _valueB, blendAlpha );
}

vec3 heightMix( vec3 _valueA, vec3 _valueB, float _alpha, float _heightA, float _heightB, float _blendWidth )
{
	//_heightA = max( 0.0, _heightA - _alpha );
	//_heightB = -(1.0-_alpha) + _heightB;

	_heightB *= _alpha;
	_heightA *= (1.0-_alpha);

	return _heightA > _heightB ? _valueA : _valueB;

	float diff = _heightB - _heightA;

	if ( diff >= 0.0 )
	{
		return mix( _valueA, _valueB, min(1.0, diff / _blendWidth) );
	}

	return mix( _valueB, _valueA, min(1.0, (-diff) / _blendWidth) );
	
	//float blendAlpha = diff * _blendWidth;
	//return mix( _valueA, _valueB, blendAlpha );
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

vec4 samplePhasedMap( sampler2D _sampler, sampler2D _heightSampler, vec2 _uv, vec2 _velocity, float _angle )
{
	_velocity = rotateBy( _velocity, _angle );

	vec2 uvA = (_uv * u_creaseMapRepeat) - u_phaseA * _velocity * u_flowOffset;
	vec2 uvB = (_uv * u_creaseMapRepeat) - u_phaseB * _velocity * u_flowOffset;

	vec4 sampleA = texture( _sampler, uvA, u_textureBias );
	vec4 sampleB = texture( _sampler, uvB, u_textureBias );

	float heightSampleA = texture( _heightSampler, uvA, u_textureBias ).x;
	float heightSampleB = texture( _heightSampler, uvB, u_textureBias ).x;

	return heightMix( sampleA, sampleB, u_phaseAlpha, heightSampleA, heightSampleB, u_hotRockRoughness );
}

vec3 samplePhasedMapNormalDXT( sampler2D _sampler, sampler2D _heightSampler, vec2 _uv, vec2 _velocity, float _angle )
{
	_velocity = rotateBy( _velocity, _angle );

	vec2 uvA = (_uv * u_creaseMapRepeat) - u_phaseA * _velocity * u_flowOffset;
	vec2 uvB = (_uv * u_creaseMapRepeat) - u_phaseB * _velocity * u_flowOffset;

	vec3 sampleA = decodeNormalDXT( texture( _sampler, uvA, u_textureBias ) );
	vec3 sampleB = decodeNormalDXT( texture( _sampler, uvB, u_textureBias ) );

	float heightSampleA = texture( _heightSampler, uvA, u_textureBias ).x;
	float heightSampleB = texture( _heightSampler, uvB, u_textureBias ).x;

	return normalize( heightMix( sampleA, sampleB, u_phaseAlpha, heightSampleA, heightSampleB, u_hotRockRoughness ) );
}

vec4 sampleBilinearPhasedMap( sampler2D _sampler, sampler2D _heightSampler, vec2 _uvTL, vec2 _uvTR, vec2 _uvBL, vec2 _uvBR, vec2 _velocity, float _angle, vec2 _ratio )
{
	vec4 sampleTL = samplePhasedMap( _sampler, _heightSampler, _uvTL, _velocity, _angle );
	vec4 sampleTR = samplePhasedMap( _sampler, _heightSampler, _uvTR, _velocity, _angle );
	vec4 sampleBL = samplePhasedMap( _sampler, _heightSampler, _uvBL, _velocity, _angle );
	vec4 sampleBR = samplePhasedMap( _sampler, _heightSampler, _uvBR, _velocity, _angle );

	return bilinearMix(sampleTL, sampleTR, sampleBL, sampleBR, _ratio);
}

vec3 sampleBilinearPhasedMapNormalDXT( sampler2D _sampler, sampler2D _heightSampler, vec2 _uvTL, vec2 _uvTR, vec2 _uvBL, vec2 _uvBR, vec2 _velocity, float _angle, vec2 _ratio )
{
	vec3 sampleTL = samplePhasedMapNormalDXT( _sampler, _heightSampler, _uvTL, _velocity, _angle );
	vec3 sampleTR = samplePhasedMapNormalDXT( _sampler, _heightSampler, _uvTR, _velocity, _angle );
	vec3 sampleBL = samplePhasedMapNormalDXT( _sampler, _heightSampler, _uvBL, _velocity, _angle );
	vec3 sampleBR = samplePhasedMapNormalDXT( _sampler, _heightSampler, _uvBR, _velocity, _angle );

	return normalize( bilinearMix(sampleTL, sampleTR, sampleBL, sampleBR, _ratio) );
}


void main(void)
{
	UpdateMousePosition();

	// Common values
	vec3 viewDir = normalize(u_cameraPos);
	vec3 lightDir = normalize(u_lightDir * u_lightDistance - in_worldPosition);

	vec3 sampledAlbedo;
	vec3 sampledNormal;
	vec3 sampledMaterial;
	float creaseAmount = 1.0;
	{
		vec2 velocity = texture( s_velocityData, in_uv ).xy * 1000;
		float clampScalar = min( 1.0, 0.2 / length(velocity) );
		velocity *= clampScalar;

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

		float blendWidth = u_hotRockRoughness;

		vec3 sampledMaterialStill = samplePhasedMap( s_lavaMaterial, s_lavaMaterial, in_uv, velocity, 0.0 ).rgb;
		vec3 sampledMaterialLong = sampleBilinearPhasedMap( s_lavaLongMaterial, s_lavaLongMaterial, uvTL, uvTR, uvBL, uvBR, velocity, angle, ratio ).rgb;
		vec3 sampledMaterialLat = sampleBilinearPhasedMap( s_lavaLatMaterial, s_lavaLatMaterial, uvTL, uvTR, uvBL, uvBR, velocity, angle, ratio ).rgb;
		sampledMaterial = mix( sampledMaterialStill, sampledMaterialLong, compressionRatio );
		sampledMaterial = mix( sampledMaterial, sampledMaterialLat, stretchRatio );
		
		vec3 sampledAlbedoStill = samplePhasedMap( s_lavaAlbedo, s_lavaMaterial, in_uv, velocity, 0.0 ).rgb;
		vec3 sampledAlbedoLong = sampleBilinearPhasedMap( s_lavaLongAlbedo, s_lavaLongMaterial, uvTL, uvTR, uvBL, uvBR, velocity, angle, ratio ).rgb;
		vec3 sampledAlbedoLat = sampleBilinearPhasedMap( s_lavaLatAlbedo, s_lavaLatMaterial, uvTL, uvTR, uvBL, uvBR, velocity, angle, ratio ).rgb;
		sampledAlbedo = mix( sampledAlbedoStill, sampledAlbedoLong, compressionRatio );
		sampledAlbedo = mix( sampledAlbedo, sampledAlbedoLat, stretchRatio );
		sampledAlbedo = pow(sampledAlbedo, vec3(2.2));
		
		vec3 sampledNormalStill = samplePhasedMapNormalDXT( s_lavaNormal, s_lavaMaterial, in_uv, velocity, 0.0 );

		vec3 sampledNormalLong = sampleBilinearPhasedMapNormalDXT( s_lavaLongNormal, s_lavaLongMaterial, uvTL, uvTR, uvBL, uvBR, velocity, angle, ratio ).xyz;
		sampledNormalLong = rotateZ( sampledNormalLong, angle );

		vec3 sampledNormalLat = sampleBilinearPhasedMapNormalDXT( s_lavaLatNormal, s_lavaLatMaterial, uvTL, uvTR, uvBL, uvBR, velocity, angle, ratio ).xyz;
		sampledNormalLat = rotateZ( sampledNormalLong, angle );

		sampledNormal = mix( sampledNormalStill, sampledNormalLong, compressionRatio );
		sampledNormal = mix( sampledNormal, sampledNormalLat, stretchRatio );

		sampledNormal = normalize(sampledNormal);
	}

	// Rock normal
	vec3 rockNormal = in_rockNormal;
	rockNormal = rotateX( rockNormal, sampledNormal.y * u_rockDetailBumpStrength ); 
	rockNormal = rotateZ( rockNormal, -sampledNormal.x * u_rockDetailBumpStrength ); 
	rockNormal = normalize(rockNormal);

	// Direct light
	float roughness = sampledMaterial.g;
	float textureAO = sampledMaterial.b;
	float reflectivity = u_rockReflectivity;

	sampledAlbedo *= max(0.0, (1.0-in_moltenAlpha*1.0));
	reflectivity *= max(0.0, (1.0-in_moltenAlpha*10.0));

	vec3 directLight = lightingGGXAlbedo( rockNormal, viewDir, lightDir, roughness, u_rockFresnelA, reflectivity, sampledAlbedo ) * u_lightIntensity * (1.0-in_shadowing) * textureAO;

	// Ambient light
	vec3 ambientLight = lightingGGXAlbedo( rockNormal, viewDir, rockNormal, roughness, u_rockFresnelB, reflectivity, sampledAlbedo ) * u_ambientLightIntensity * in_occlusion * textureAO;

	// Local glow from heat
	vec3 heatLight = vec3(0.0);
	{
		vec3 sampleOffset = normalize( vec3( rockNormal.x, 0.0, rockNormal.z ) );
		
		vec3 samplePos = in_worldPosition + sampleOffset * u_glowDistance;
		vec2 sampleUV = samplePos.xz + vec2(0.5);
		vec4 sampleHeightData = texture(s_heightData, sampleUV);
		samplePos.y = sampleHeightData.x + sampleHeightData.y + sampleHeightData.z;

		float sampleHeat = textureLod( s_miscData, sampleUV, u_glowMipLevel ).x;
		vec3 sampleDir = samplePos - in_worldPosition;
		float sampleDis = length(sampleDir);
		sampleDir = normalize(sampleDir);

		float sampleHeatLight = lightingGGX( rockNormal, viewDir, sampleDir, roughness, u_rockFresnelA );
		sampleHeatLight *= max( 0.0, sampleHeat - 0.1 );
		sampleHeatLight /= 1.0 + sampleDis*sampleDis;

		heatLight += sampleHeatLight;
		heatLight *=  vec3(1.0,0.1,0.0) * u_glowScalar * textureAO;
	}

	// Bring it all together
	vec3 outColor = directLight + ambientLight + heatLight;

	// Add emissve elements
	float moltenAlpha = mix( max( in_moltenAlpha - (sampledMaterial.r * sampledMaterial.b), 0.0 ), in_moltenAlpha * (1.0 - sampledMaterial.r * sampledMaterial.b), 0.3 );

	outColor = mix( outColor, in_moltenColor, moltenAlpha * sampledMaterial.g );
	
	out_viewPosition = in_viewPosition;
	out_forward = vec4( outColor, 1.0 );
}
