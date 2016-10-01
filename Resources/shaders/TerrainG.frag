#version 430 core


////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Varying
in Varying
{
	vec4 in_viewPosition;
	vec4 in_forward;
	vec2 in_uv;
	float in_dirtAlpha;
	vec3 in_moltenColor;
	float in_moltenAlpha;
	vec3 in_rockNormal;
	float in_occlusion;
	float in_heat;
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

uniform vec3 u_hotRockColorA;
uniform vec3 u_hotRockColorB;
uniform float u_hotRockRoughnessA;
uniform float u_hotRockRoughnessB;
uniform float u_hotRockFresnelA;
uniform float u_hotRockFresnelB;

uniform float u_moltenAlphaPower;

uniform vec3 u_cameraPos;
uniform float u_rockDetailDiffuseStrength;
uniform float u_rockDetailBumpStrength;

uniform vec3 u_lightDir;
uniform float u_lightIntensity;
uniform float u_ambientLightIntensity;

// Textures
uniform sampler2D s_rockDiffuse;
uniform sampler2D s_moltenMapData;
uniform sampler2D s_smudgeData;


////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_viewPosition;
layout( location = 1 ) out vec4 out_viewNormal;
layout( location = 2 ) out vec4 out_albedo;
layout( location = 3 ) out vec4 out_material;
layout( location = 4 ) out vec4 out_forward;

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

void main(void)
{
	UpdateMousePosition();

	// Common values
	vec3 viewDir = normalize(u_cameraPos);
	float moltenMapValue = texture(s_moltenMapData, in_uv).x;

	vec3 mippedMoltenMapValue = texture( s_moltenMapData, in_uv ).xyz;

	float powedMoltenMapValue = pow(moltenMapValue, mix( 1.0, 0.1, in_rockNormal.y ));



	// Rock material
	vec3 rockDiffuse = pow( mix( u_rockColorA, u_rockColorB, powedMoltenMapValue ), vec3(2.2) );
	float rockRoughness = mix( u_rockRoughnessA, u_rockRoughnessB, powedMoltenMapValue );
	float rockFresnel = mix( u_rockFresnelA, u_rockFresnelB, powedMoltenMapValue );

	// Hot rock material
	vec3 hotRockDiffuse = pow( mix( u_hotRockColorA, u_hotRockColorB, powedMoltenMapValue ), vec3(2.2) );
	float hotRockRoughness = mix( u_hotRockRoughnessA, u_hotRockRoughnessB, powedMoltenMapValue );
	float hotRockFresnel = mix( u_hotRockFresnelA, u_hotRockFresnelB, powedMoltenMapValue );
	
	// Mix rock and hot rock together
	float hotRockMaterialLerp = min( in_heat / 0.3, 1.0 );

	vec3 diffuse = mix( rockDiffuse, hotRockDiffuse, hotRockMaterialLerp );
	float roughness = mix( rockRoughness, hotRockRoughness, hotRockMaterialLerp );
	float fresnel = mix( rockFresnel, hotRockFresnel, hotRockMaterialLerp );
	
	// Detail
	vec4 rockDiffuseSample = texture( s_rockDiffuse, in_uv );
	diffuse *= 1.0 - ((1.0-rockDiffuseSample.b) * u_rockDetailDiffuseStrength);

	vec3 rockNormal = in_rockNormal;
	vec2 angle = mippedMoltenMapValue.yz * mix( u_rockDetailBumpStrength, u_rockDetailBumpStrength * 0.2, pow(in_rockNormal.y, 4.0) );
	rockNormal = rotateX( rockNormal, -angle.x ); 
	rockNormal = rotateZ( rockNormal, angle.y ); 
	rockNormal = normalize(rockNormal);

	// Direct light
	float directLight = lightingGGX( rockNormal, viewDir, u_lightDir, roughness, fresnel ) * u_lightIntensity;

	// Ambient light
	float ambientlight = lightingGGX( rockNormal, viewDir, vec3(0.0,1.0,0.0), 1.0, fresnel ) * u_ambientLightIntensity * in_occlusion;

	// Bring it all together
	vec3 outColor = (diffuse * (directLight + ambientlight));
	float moltenAlpha = in_moltenAlpha * (1.0-moltenMapValue);
	outColor = mix( outColor, in_moltenColor * (1.0-pow(clamp(moltenMapValue,0.0,1.0), u_moltenAlphaPower)), moltenAlpha );
	
	// Heat glow
	float heatGlowAlpha = max( pow(1.0-moltenMapValue, 2.0) * hotRockMaterialLerp * 0.4, 0.0f);
	outColor += in_moltenColor * heatGlowAlpha;
	outColor += in_moltenColor * pow( heatGlowAlpha, 4.0 ) * 5000.0;

	//outColor *= 0.1;
	//outColor += vec3( texture(s_smudgeData, in_uv).z ) * 0.5;

	out_forward = vec4( outColor, 1.0 );
	out_viewPosition = vec4(0.0);
	out_viewNormal = vec4(0.0);
	out_albedo = vec4(0.0);
	out_material = vec4(0.0);
}
