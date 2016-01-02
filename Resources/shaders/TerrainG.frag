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

	ivec2 cellIndex = ivec2( in_uv * u_numCells );

	vec4 uvC = texelFetch(s_mappingData, cellIndex, 0 );
	float textureScalar = uvC.y;

	float diffuseScalar = texture(s_diffuseMap, vec2(uvC.x,0)).z;

	float heat = in_data0.z;

	vec3 diffuse = 0.05f + vec3(diffuseScalar) * 0.05f;
	// Scortch the diffuse
	diffuse = max(vec3(0.0f), diffuse-max(0.0f,heat-0.1f)*0.1f);

	float occlusion = 1.0f - in_normal.w;

	// Direct light
	vec3 directLight = vec3(0.0f);
	vec3 lightDir = normalize(vec3(1.0,1.0f,1.0f));
	directLight += max( dot(in_normal.xyz, lightDir), 0.0f ) * 2.0f;

	// Ambient light
	vec3 ambientlight = vec3(0.0f);
	ambientlight += 0.75f * occlusion;

	// Emissive
	textureScalar = pow(textureScalar, 2.0f);
	

	float heatForColor0 = min(1.0f,heat);
	float heatColor0 = max(0.0f, heatForColor0 - textureScalar * 1.0f );
	heatColor0 += pow( heatForColor0, 1.5f ) * 0.25f;

	float heatForColor1 = max(heat-0.1f, 0.0f) * 0.75f;
	float heatColor1 = max(0.0f, heatForColor1 - textureScalar * 1.0f);
	heatColor1 = pow(heatColor1, 1.5f);

	heatColor0 += heatColor1;

	vec3 emissive = vec3(0.0f);
	emissive.y = heatColor0;
	emissive.z = heatColor1;

	vec3 outColor = (diffuse * (directLight + ambientlight)) + emissive;

	//outColor = vec3(textureScalar);
	//outColor += ambientlight * 0.1f;

	out_viewPosition = vec4(outColor,1.0f);
	out_viewNormal = vec4( in_viewNormal.xyz, 1.0f );
	out_albedo = vec4( 1.0f );
	out_material = vec4( 1.0f, 1.0f, 0.0f, 1.0f );	// roughness, reflectivity, emissive, nowt
	out_directLight = vec4( 0.0f );
}