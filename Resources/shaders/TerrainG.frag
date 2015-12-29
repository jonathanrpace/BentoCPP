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

// Textures
uniform sampler2D s_diffuseMap2;
uniform sampler2D s_mappingData2;


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

	ivec2 cellIndex = ivec2( in_uv * u_numCells );

	vec4 diffuseSample;
	if ( false )
	{
		vec2 uv0 = texelFetch(s_mappingData2, cellIndex, 0 ).xy;
		vec2 uv1 = texelFetch(s_mappingData2, cellIndex + ivec2(1,0), 0 ).xy;
		vec2 uv2 = texelFetch(s_mappingData2, cellIndex + ivec2(0,1), 0 ).xy;
		vec2 uv3 = texelFetch(s_mappingData2, cellIndex + ivec2(1,1), 0 ).xy;

		vec2 uvPerCell = 1.0f / u_numCells;
		vec2 uvRatios = mod(in_uv / uvPerCell, 1.0f);

		uv0 += uvRatios * uvPerCell;
		uv1.x -= (1.0f - uvRatios.x) * uvPerCell.x;
		uv1.y += uvRatios.y * uvPerCell.y;
		uv2.x += uvRatios.x * uvPerCell.x;
		uv2.y -= (1.0f - uvRatios.y) * uvPerCell.y;
		uv3 -= (1.0f-uvRatios) * uvPerCell;

		// Use 4 samples
		vec4 diffuseSample0 = texture( s_diffuseMap2, uv0 );
		vec4 diffuseSample1 = texture( s_diffuseMap2, uv1 );
		vec4 diffuseSample2 = texture( s_diffuseMap2, uv2 );
		vec4 diffuseSample3 = texture( s_diffuseMap2, uv3 );

		diffuseSample = diffuseSample0 * (1.0f-uvRatios.x) * (1.0f-uvRatios.y);
		diffuseSample += diffuseSample1 * uvRatios.x * (1.0f-uvRatios.y);
		diffuseSample += diffuseSample2 * (1.0f-uvRatios.x) * uvRatios.y;
		diffuseSample += diffuseSample3 * uvRatios.x * uvRatios.y;
	}
	else
	{
		vec2 uv0 = texelFetch(s_mappingData2, ivec2(cellIndex), 0 ).xy;
		diffuseSample = texture( s_diffuseMap2, uv0 );
	}
	
	float heat = in_data0.z;
	
	vec3 diffuse = 0.05f + vec3(diffuseSample.x) * 0.05f;
	
	// Scortch the diffuse
	diffuse = max(vec3(0.0f), diffuse-max(0.0f,heat-0.1f)*0.1f);

	// Direct light
	vec3 directLight = vec3(0.0f);
	vec3 lightDir = normalize(vec3(0.0,1.0f,0.0f));
	directLight += max( dot(in_normal.xyz, lightDir), 0.0f ) * 1.5;

	// Ambient light
	vec3 ambientlight = vec3(0.0f);
	float occlusion = 1.0f - in_normal.w;
	ambientlight += 0.75f * occlusion;

	// Emissive
	vec3 emissive = vec3(0.0f);
	float oneMinusClampedHeat = 1.0f-clamp(heat,0.0f,1.0f);
	float heatColor0 = max(0.0f, heat - (diffuseSample.x) * 0.5f);
	emissive.x += pow( min(heatColor0, 1.0f), 1.5f );

	float heatColor1 = max(0.0f, heat - (diffuseSample.x) * 0.8f);
	emissive.y += pow( max(0.0f, heatColor1-0.6f), 0.8f );


	vec3 outColor = (diffuse * (directLight + ambientlight)) + emissive;

	out_viewPosition = vec4(outColor,1.0f);
	out_viewNormal = vec4( in_viewNormal.xyz, 1.0f );
	out_albedo = vec4( 1.0f );
	out_material = vec4( 1.0f, 1.0f, 0.0f, 1.0f );	// roughness, reflectivity, emissive, nowt
	out_directLight = vec4( 0.0f );
}