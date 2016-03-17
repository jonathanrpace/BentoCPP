#version 430 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Varying
in Varying
{
	vec2 in_uv;
	vec3 in_viewNormal;
	vec4 in_viewPosition;
	vec4 in_rockData;
	vec4 in_waterData;
	vec4 in_rockNormal;
	vec4 in_waterNormal;
	vec4 in_waterFluxData;
};

// Uniforms
uniform ivec2 u_numCells;
uniform vec2 u_mouseScreenPos;
uniform ivec2 u_windowSize;

uniform mat4 u_viewMatrix;
uniform vec3 u_lightDir;
uniform float u_lightIntensity;
uniform float u_ambientLightIntensity;

uniform vec3 u_fogColorAway;
uniform vec3 u_fogColorTowards;
uniform float u_fogHeight;
uniform float u_fogDensity;
uniform vec3 u_cameraPos;

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
// Functions
////////////////////////////////////////////////////////////////

vec3 ApplyFog( in vec3  rgb,      // original color of the pixel
               in float distance, // camera to point distance
               in vec3  rayDir,   // camera to point vector
			   in vec3  cameraPos,
               in vec3  sunDir )  // sun light direction
{
    float fogAmount = u_fogDensity * exp(-cameraPos.y*u_fogHeight) * (1.0-exp( -distance*rayDir.y*u_fogHeight ))/rayDir.y;
    float sunDot = max( dot( rayDir, sunDir ), 0.0 );
    vec3  fogColor  = mix( u_fogColorAway,
                           u_fogColorTowards,
                           pow(sunDot,8.0) );
    return mix( rgb, fogColor, fogAmount );
}

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

void main(void)
{
	UpdateMousePosition();

	vec4 mappingDataC = texture(s_mappingData, in_uv, 0 );

	float moltenPhase = mappingDataC.x;
	float moltenBump = mappingDataC.y;
	float heat = in_rockData.z;
	float occlusion = 1.0f - in_rockNormal.w;

	// Diffuse
	float diffuseScalar = moltenPhase;//texture(s_diffuseMap, vec2(moltenPhase,0)).x;
	vec3 diffuse = vec3(0.05f);// + vec3(diffuseScalar) * 0.05f;
	diffuse = max(vec3(0.0f), diffuse-max(0.0f,heat-0.0f)*0.05f);		// Scortch

	// Direct light
	vec3 directLight = vec3( max( dot(in_rockNormal.xyz, u_lightDir), 0.0f ) * u_lightIntensity );

	// Ambient light
	vec3 ambientlight = vec3( u_ambientLightIntensity * occlusion );

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

	outColor = ApplyFog( outColor, length(in_viewPosition.xyz), -normalize(in_viewPosition.xyz), u_cameraPos, -u_lightDir );

	outColor = u_cameraPos;//normalize(in_viewPosition.xyz);

	out_viewPosition = in_viewPosition;
	out_viewNormal = vec4( normalize( in_waterNormal.xyz * mat3(u_viewMatrix) ), 1.0f );
	out_albedo = vec4( 0.0f );
	out_material = vec4( 0.0f, 0.0f, 0.0f, 0.0f );	// roughness, reflectivity, emissive, nowt
	out_forward = vec4(outColor, 1.0f);
}