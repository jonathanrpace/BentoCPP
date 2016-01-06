#version 430 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Varying
in Varying
{
	vec2 in_uv;
	vec4 in_viewNormal;
	vec4 in_viewPosition;
	vec4 in_data0;
	vec4 in_mappingData;
	vec4 in_data2;
	vec4 in_screenPos;
	vec4 in_normal;
};

// Uniforms
uniform ivec2 u_numCells;
uniform vec2 u_mouseScreenPos;
uniform ivec2 u_windowSize;

uniform vec3 u_lightDir = vec3(1.0,1.0f,1.0f);
uniform float u_lightStrength = 2.0f;
uniform float u_ambientLightStrength = 0.75f;

// Textures
uniform sampler2D s_diffuseMap;
uniform sampler2D s_mappingData;


////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_viewPosition;
layout( location = 1 ) out vec4 out_viewNormal;
layout( location = 2 ) out vec4 out_albedo;
layout( location = 3 ) out vec4 out_material;
layout( location = 4 ) out vec4 out_directLight;

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
// Functions
////////////////////////////////////////////////////////////////

void UpdateMousePosition()
{
	int fragViewZ = int(-in_viewPosition.z * 256.0f);

	vec4 screenPos = in_screenPos / in_screenPos.w;
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

void main(void)
{
	UpdateMousePosition();

	vec4 mappingDataC = texture(s_mappingData, in_uv, 0 );

	float moltenPhase = mappingDataC.x;
	float moltenBump = mappingDataC.y;
	float heat = in_data0.z;
	float occlusion = 1.0f - in_normal.w;

	// Diffuse
	float diffuseScalar = texture(s_diffuseMap, vec2(moltenPhase,0)).y;
	vec3 diffuse = 0.05f + vec3(diffuseScalar) * 0.05f;
	diffuse = max(vec3(0.0f), diffuse-max(0.0f,heat-0.1f)*0.05f);		// Scortch

	// Direct light
	vec3 directLight = vec3( max( dot(in_normal.xyz, normalize(u_lightDir)), 0.0f ) * u_lightStrength );

	// Ambient light
	vec3 ambientlight = vec3( u_ambientLightStrength * occlusion );

	// Emissive
	vec3 emissive = vec3(0.0f);
	float heatForColor0 = min(1.0f,heat);
	float heatColor0 = max(0.0f, heatForColor0 - moltenBump * 0.5f );
	heatColor0 = pow(heatColor0, 2.0f);

	float heatForColor1 = heat * 0.8f;
	float heatColor1 = max(0.0f, heatForColor1 - moltenBump * 0.25f);
	heatColor1 = min(1.0f,heatColor1);
	heatColor1 = pow(heatColor1, 5.0f);

	heatColor0 += heatColor1;

	emissive.x = heatColor0;
	emissive.y = heatColor1;
	
	// Bing it all together
	vec3 outColor = (diffuse * (directLight + ambientlight)) + emissive;

	out_viewPosition = vec4(outColor, 1.0f);
	out_viewNormal = vec4( in_viewNormal.xyz, 1.0f );
	out_albedo = vec4( 1.0f );
	out_material = vec4( 1.0f, 1.0f, 0.0f, 1.0f );	// roughness, reflectivity, emissive, nowt
	out_directLight = vec4( 0.0f );
}