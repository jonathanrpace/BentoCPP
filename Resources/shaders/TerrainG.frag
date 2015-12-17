#version 330 core

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
	vec4 in_diffuse;
	vec4 in_normal;
};

// Uniforms
uniform vec2 u_numCells;

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
// Functions
////////////////////////////////////////////////////////////////

void main(void)
{
	vec2 uvPerCell = 1.0f / u_numCells;
	vec2 uvRatios = mod(in_uv / uvPerCell, 1.0f);
	vec2 cellIndex = floor(in_uv / uvPerCell);

	vec2 uv0 = texture(s_mappingData2, cellIndex * uvPerCell ).xy;
	vec2 uv1 = texture(s_mappingData2, (cellIndex + vec2(1,0)) * uvPerCell ).xy;
	vec2 uv2 = texture(s_mappingData2, (cellIndex + vec2(0,1)) * uvPerCell ).xy;
	vec2 uv3 = texture(s_mappingData2, (cellIndex + vec2(1,1)) * uvPerCell ).xy;

	uv0 += uvRatios * uvPerCell;

	uv1.x -= (1.0f - uvRatios.x) * uvPerCell.x;
	uv1.y += uvRatios.y * uvPerCell.y;

	uv2.x += uvRatios.x * uvPerCell.x;
	uv2.y -= (1.0f - uvRatios.y) * uvPerCell.y;
	
	uv3 -= (1.0f-uvRatios) * uvPerCell;

	vec4 diffuseSample0 = texture( s_diffuseMap2, uv0 );
	vec4 diffuseSample1 = texture( s_diffuseMap2, uv1 );
	vec4 diffuseSample2 = texture( s_diffuseMap2, uv2 );
	vec4 diffuseSample3 = texture( s_diffuseMap2, uv3 );

	vec4 diffuseSample = diffuseSample0 * (1.0f-uvRatios.x) * (1.0f-uvRatios.y);
	diffuseSample += diffuseSample1 * uvRatios.x * (1.0f-uvRatios.y);
	diffuseSample += diffuseSample2 * (1.0f-uvRatios.x) * uvRatios.y;
	diffuseSample += diffuseSample3 * uvRatios.x * uvRatios.y;

	
	//mix( mix(diffuseSample0, diffuseSample1, uvRatios.x), diffuseSample2, uvRatios.y );
	
	float heat = in_data0.z;

	vec3 outColor = vec3( clamp( dot(in_normal.xyz, normalize(vec3(0.5,1.0f,0.2f))), 0.0f, 1.0f ) );
	outColor *= 0.2f;

	//outColor.xyz += abs(in_data2.xyz);

	//outColor.z += clamp( in_data0.y / in_data0.x, 0.0f, 1.0f );

	float heatForColor = max(0.0f, heat - (diffuseSample.y) * 0.5f);
	outColor.x += pow( min(heatForColor, 1.0f), 1.5f );

	float heatForColor2 = max(0.0f, heat - (diffuseSample.y) * 0.8f);
	outColor.y += pow( max(0.0f, heatForColor2-0.6f), 0.8f );

	out_viewPosition = vec4(outColor,1.0f);
	out_viewNormal = vec4( in_viewNormal.xyz, 1.0f );
	out_albedo = vec4( 1.0f );
	out_material = vec4( 1.0f, 1.0f, 0.0f, 1.0f );	// roughness, reflectivity, emissive, nowt
	out_directLight = vec4( 0.0f );

	//out_viewPosition = diffuseSample;
}