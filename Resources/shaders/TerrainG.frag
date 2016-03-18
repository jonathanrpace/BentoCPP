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
uniform float u_fogFalloff;
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
/*
vec3 rayDir = pos - vCamPosition;
float dist = length(rayDir); // distance is a keyword in GLSL
 
const float maxFogHeight = 900; // higher the fog won't go
const float c = 0.1;
const float b = 0.2;
 
// if this is not done, the result will look awful
if(pos.z >= maxFogHeight - 1 / c)
{		
    return;
}
 
// distance in fog is calculated with a simple intercept theorem
float distInFog = dist * (maxFogHeight - pos.z) / (pos.z - vCamPosition.z);
 
// when dist is 0, log(dist) is 1, so subtract this
float fogAmount = (log(distInFog * c) - 1) * b;
 
// at the top border, the value can get greater than 1, so clamp
fogAmount = clamp(fogAmount, 0, 1);
 
// final mix of colors
gl_FragColor = mix(gl_FragColor, fogColor, fogAmount);

*/

vec3 ApplyFog( in vec3  rgb,      // original color of the pixel
               in float dist, // camera to point distance
               in vec3  rayDir,   // camera to point vector
			   in vec3  cameraPos,
			   in vec3  fragPos,
               in vec3  sunDir )  // sun light direction
{
	// if this is not done, the result will look awful
	if(fragPos.y >= u_fogHeight - 1 / u_fogFalloff)
	{		
		return rgb;
	}

	float sunDot = max( dot( rayDir, sunDir ), 0.0 );
    vec3  fogColor  = mix( u_fogColorAway,
                           u_fogColorTowards,
                           pow(sunDot,4.0) );

	float distInFog = dist * (u_fogHeight - fragPos.y) / (cameraPos.y-fragPos.y);
	float fogAmount = (log(distInFog * u_fogFalloff) - 1) * u_fogDensity;
	fogAmount = clamp(fogAmount, 0, 1);

    //float fogAmount = exp(-cameraPos.y*u_fogDensity);
	//fogAmount = (1.0-exp( -dist*-rayDir.y )) / (-rayDir.y*u_fogHeight);

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


	mat4 invViewMatrix = inverse(u_viewMatrix);
	vec3 worldPosition = vec3( invViewMatrix * vec4(in_viewPosition.xyz,1) );
	vec3 cameraRay = worldPosition-u_cameraPos;
	outColor = ApplyFog( outColor, length(worldPosition-u_cameraPos), cameraRay, u_cameraPos, worldPosition, u_lightDir );

	//outColor = vec3(cameraRay.y);

	out_viewPosition = in_viewPosition;
	out_viewNormal = vec4( normalize( in_waterNormal.xyz * mat3(u_viewMatrix) ), 1.0f );
	out_albedo = vec4( 0.0f );
	out_material = vec4( 0.0f, 0.0f, 0.0f, 0.0f );	// roughness, reflectivity, emissive, nowt
	out_forward = vec4(outColor, 1.0f);
}