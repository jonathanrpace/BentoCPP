#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Varying
in Varying
{
	vec4 in_viewNormal;
	vec4 in_viewPosition;
	vec4 in_data0;
	vec4 in_data1;
	vec4 in_data2;
	vec4 in_diffuse;
	vec4 in_normal;
};

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
	float heat = in_data0.z;

	vec3 outColor = vec3( clamp( dot(in_normal.xyz, normalize(vec3(0.5,1.0f,0.2f))), 0.0f, 1.0f ) );
	
	outColor *= 0.2f;

	//outColor.xyz += abs(in_data2.xyz);

	float heatForColor = max(0.0f, heat - (1.0f-in_diffuse.y) * 0.5f);
	outColor.x += heatForColor * 0.75;
	outColor.y += max(0.0f, heatForColor-1.0f);

	out_viewPosition = vec4(outColor,1.0f);
	out_viewNormal = vec4( in_viewNormal.xyz, 1.0f );
	out_albedo = vec4( 1.0f );
	out_material = vec4( 1.0f, 1.0f, 0.0f, 1.0f );	// roughness, reflectivity, emissive, nowt
	out_directLight = vec4( 0.0f );
}