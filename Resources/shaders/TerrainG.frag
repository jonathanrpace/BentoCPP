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

uniform vec3 u_rockColorA;
uniform vec3 u_rockColorB;
uniform float u_rockRoughnessA;
uniform float u_rockRoughnessB;
uniform float u_rockFresnelA;
uniform float u_rockFresnelB;

uniform vec3 u_hotRockColor;
uniform float u_hotRockRoughness;
uniform float u_hotRockFresnel;

uniform float u_moltenPlateAlpha;
uniform float u_moltenPlateAlphaPower;
uniform float u_moltenCreaseAlpha;
uniform float u_moltenCreaseAlphaPower;

uniform vec3 u_moltenColor;
uniform float u_moltenColorScalar;

uniform float u_glowScalar;
uniform float u_glowPower;
uniform float u_glowDetailScalar;
uniform float u_glowDetailPower;

uniform vec3 u_dirtColor;

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
uniform sampler2D s_rockDiffuse;
uniform sampler2D s_creaseMap;
uniform sampler2D s_moltenMapData;
uniform sampler2D s_smudgeData;
uniform sampler2D s_velocityData;
uniform sampler2D s_miscData;
uniform sampler2D s_heightData;

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
vec4 sampleCombinedMip( sampler2D _sampler, vec2 _uv, int _minMip, int _maxMip, float _downSampleScalar );

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

float sampleCreaseMapLong( vec2 _uv, vec2 _velocity, float _angle )
{
	_velocity = rotateBy( _velocity, _angle );

	vec2 uvA = (_uv * u_creaseMapRepeat) - u_phaseA * _velocity * u_flowOffset;
	vec2 uvB = (_uv * u_creaseMapRepeat) - u_phaseB * _velocity * u_flowOffset;

	float sampleA = texture( s_creaseMap, uvA ).x;
	float sampleB = texture( s_creaseMap, uvB ).x;

	return mix( sampleA, sampleB, u_phaseAlpha );
}

float sampleCreaseMapLat( vec2 _uv, vec2 _velocity, float _angle )
{
	_velocity = rotateBy( _velocity, _angle );

	vec2 uvA = (_uv * u_creaseMapRepeat) - u_phaseA * _velocity * u_flowOffset;
	vec2 uvB = (_uv * u_creaseMapRepeat) - u_phaseB * _velocity * u_flowOffset;

	float sampleA = texture( s_creaseMap, uvA ).y;
	float sampleB = texture( s_creaseMap, uvB ).y;

	return mix( sampleA, sampleB, u_phaseAlpha );
}

float sampleCreaseMapStill( vec2 _uv, vec2 _velocity, float _angle )
{
	//_velocity = rotateBy( _velocity, _angle );

	vec2 uvA = (_uv * u_creaseMapRepeat) - u_phaseA * _velocity * u_flowOffset;
	vec2 uvB = (_uv * u_creaseMapRepeat) - u_phaseB * _velocity * u_flowOffset;

	float sampleA = texture( s_creaseMap, uvA ).z;
	float sampleB = texture( s_creaseMap, uvB ).z;

	return mix( sampleA, sampleB, u_phaseAlpha );
}

void main(void)
{
	UpdateMousePosition();

	// Common values
	vec3 viewDir = normalize(u_cameraPos);
	vec4 moltenMapSample = texture(s_moltenMapData, in_uv);
	float moltenMapValue = clamp( moltenMapSample.x, 0.0, 1.0 );

	float powedMoltenMapValue = pow(moltenMapValue, mix( 1.0, 0.1, in_rockNormal.y ));

	vec3 lightDir = normalize(u_lightDir * u_lightDistance - in_worldPosition);

	// Creases
	float creaseAmount = 1.0;
	{
		vec2 velocity2 = texture( s_velocityData, in_uv ).xy * 1000;

		vec2 velocity = textureLod( s_smudgeData, in_uv, u_creaseMipLevel ).xy;
		vec2 forwardVelocity = textureLod( s_smudgeData, in_uv + normalize(velocity) * u_creaseForwardScalar, u_creaseMipLevel ).xy;

		float projectionLength = dot( forwardVelocity, velocity / length(velocity) );
		projectionLength -= length(velocity) * u_creaseRatio;

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

		float angle = -atan(velocity.y, velocity.x );

		vec2 creaseUV_TL = in_uv-gridCellCenterTL;
		creaseUV_TL = rotateBy( creaseUV_TL, angle );
		creaseUV_TL += gridCellCenterTL;
		float creaseSampleLongTL = sampleCreaseMapLong( creaseUV_TL, velocity2, angle );
		float creaseSampleLatTL = sampleCreaseMapLat( creaseUV_TL, velocity2, angle );

		vec2 creaseUV_TR = in_uv-gridCellCenterTR;
		creaseUV_TR = rotateBy( creaseUV_TR, angle );
		creaseUV_TR += gridCellCenterTR;
		float creaseSampleLongTR = sampleCreaseMapLong( creaseUV_TR, velocity2, angle );
		float creaseSampleLatTR = sampleCreaseMapLat( creaseUV_TR, velocity2, angle );

		vec2 creaseUV_BL = in_uv-gridCellCenterBL;
		creaseUV_BL = rotateBy( creaseUV_BL, angle );
		creaseUV_BL += gridCellCenterBL;
		float creaseSampleLongBL = sampleCreaseMapLong( creaseUV_BL, velocity2, angle );
		float creaseSampleLatBL = sampleCreaseMapLat( creaseUV_BL, velocity2, angle );

		vec2 creaseUV_BR = in_uv-gridCellCenterBR;
		creaseUV_BR = rotateBy( creaseUV_BR, angle );
		creaseUV_BR += gridCellCenterBR;
		float creaseSampleLongBR = sampleCreaseMapLong( creaseUV_BR, velocity2, angle );
		float creaseSampleLatBR = sampleCreaseMapLat( creaseUV_BR, velocity2, angle );
		
		// Bilinear interpolation
		float creaseSampleLong = ratio.x * ratio.y * creaseSampleLongBR;
		creaseSampleLong += (1.0-ratio.x) * ratio.y * creaseSampleLongBL;
		creaseSampleLong += ratio.x * (1.0-ratio.y) * creaseSampleLongTR;
		creaseSampleLong += (1.0-ratio.x) * (1.0-ratio.y) * creaseSampleLongTL;

		float creaseSampleLat = ratio.x * ratio.y * creaseSampleLatBR;
		creaseSampleLat += (1.0-ratio.x) * ratio.y * creaseSampleLatBL;
		creaseSampleLat += ratio.x * (1.0-ratio.y) * creaseSampleLatTR;
		creaseSampleLat += (1.0-ratio.x) * (1.0-ratio.y) * creaseSampleLatTL;

		creaseAmount = sampleCreaseMapStill( in_uv, velocity2, angle );
		creaseAmount = mix( creaseAmount, creaseSampleLong, stretchRatio );
		creaseAmount = mix( creaseAmount, creaseSampleLat, compressionRatio );

		creaseAmount = clamp(creaseAmount, 0.0, 1.0);

		//outColor = pow( vec3( outCrease ), vec3(2.2) );

		//outColor = vec3(velocity2 * 1000, 0.0);
		//outColor = pow( vec3( gridCellCenterC, 0.0), vec3(2.2) );
		//outColor = vec3(max(0, -projectionLength), max(0, projectionLength), 0.0) * u_bearingCreaseScalar;
	}

	//powedMoltenMapValue -= creaseAmount;
	//moltenMapValue -= creaseAmount * 0.5;

	float diffuseRatio = moltenMapValue;

	// Rock material
	vec3 rockDiffuse = pow( mix( u_rockColorA, u_rockColorB, diffuseRatio ), vec3(2.2) );
	float rockRoughness = mix( u_rockRoughnessA, u_rockRoughnessB, diffuseRatio );
	float rockFresnel = mix( u_rockFresnelA, u_rockFresnelB, diffuseRatio );

	// Mix rock and hot rock together
	float hotRockMaterialLerp = min( in_heat / 0.05, 1.0 );
	vec3 diffuse = mix( rockDiffuse, pow( u_hotRockColor, vec3(2.2) ), hotRockMaterialLerp );
	float roughness = mix( rockRoughness, u_hotRockRoughness, hotRockMaterialLerp );
	float fresnel = mix( rockFresnel, u_hotRockFresnel, hotRockMaterialLerp );
	
	diffuse *= mix( 1.0, creaseAmount, 0.8 );

	// Rock normal
	vec3 rockNormal = in_rockNormal;
	float slopeScalar = pow(in_rockNormal.y, u_rockDetailBumpSlopePower);
	float bumpLod = -0.5 + in_dirtAlpha * 7 + in_rockNormal.y * 2.0;
	vec2 moltenMapDerivative = textureLod( s_moltenMapData, in_uv, bumpLod ).yz;

	// Normal detail
	vec2 angle = moltenMapDerivative * u_rockDetailBumpStrength * max(1.0-slopeScalar, 0.2);
	rockNormal = rotateX( rockNormal, -angle.x ); 
	rockNormal = rotateZ( rockNormal, angle.y ); 
	rockNormal = normalize(rockNormal);

	// Dirt
	vec3 dirtDiffuse = pow(u_dirtColor, vec3(2.2));
	diffuse = mix( diffuse, dirtDiffuse, in_dirtAlpha );

	// Local glow from heat
	vec3 heatLight = vec3(0.0);
	{
		float dis = 0.2;
		int mipLevel = 2;
		float strength = 1.0;
		float totalStrength = 0.0;
		for ( int i = 0; i < 3; i++ )
		{
			vec3 samplePos = in_worldPosition + rockNormal * dis;
			vec2 sampleUV = samplePos.xz + vec2(0.5);
			vec4 sampleHeightData = texture(s_heightData, sampleUV);
			samplePos.y = sampleHeightData.x + sampleHeightData.y + sampleHeightData.z;
			
			float sampleHeat = textureLod( s_miscData, sampleUV, mipLevel ).x;
			vec3 sampleDir = normalize( samplePos - in_worldPosition );

			totalStrength += strength;
			dis *= 2.0;
			strength *= 0.5;
			mipLevel++;

			float sampleHeatLight = lightingGGX( rockNormal, viewDir, sampleDir, roughness, fresnel );
			sampleHeatLight *= max( 0.0, sampleHeat-0.1) * (dot( rockNormal, sampleDir ) + 1.0) * 0.5;

			heatLight += sampleHeatLight;
		}

		heatLight /= totalStrength;

		//heatLight = lightingGGX( normalize(rockNormal + sampleDir), viewDir, sampleDir, roughness, fresnel );
		heatLight *=  vec3(1.0,0.1,0.0) * 40.0;// * in_occlusion;
	}

	// Direct light
	float directLight = lightingGGX( rockNormal, viewDir, lightDir, roughness, fresnel ) * u_lightIntensity * (1.0-in_shadowing);

	// Ambient light
	float ambientlight = lightingGGX( rockNormal, viewDir, rockNormal, roughness, fresnel ) * u_ambientLightIntensity * in_occlusion;

	// Bring it all together
	vec3 outColor = (diffuse * (directLight + ambientlight)) + diffuse * heatLight;

	//outColor = vec3( vec3(in_worldPosition + rockNormal * 0.05).xz, 0.0 );

	// Add emissve elements
	float moltenPlateScalar = 1.0 - (pow(moltenMapValue, u_moltenPlateAlphaPower) * u_moltenPlateAlpha);
	float moltenCreaseScalar = 1.0 - (pow(creaseAmount, u_moltenCreaseAlphaPower) * u_moltenCreaseAlpha);
	float moltenAlpha = in_moltenAlpha * moltenPlateScalar * moltenCreaseScalar;
	outColor = mix( outColor, in_moltenColor, moltenAlpha );
	
	// Heat glow
	float heatGlowAlpha = pow(1.0-moltenMapValue, u_glowPower) * in_heat * u_glowScalar;
	outColor += in_moltenColor * heatGlowAlpha * (1.0-moltenAlpha);

	float heatGlowDetailAlpha = pow(1.0-creaseAmount, u_glowDetailPower) * in_heat * u_glowDetailScalar;
	outColor += in_moltenColor * heatGlowDetailAlpha * (1.0-moltenAlpha);

	
	out_viewPosition = in_viewPosition;
	out_forward = vec4( outColor, 1.0 );
}
