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


vec3 ApplyFog( vec3 rgb, vec3 c, vec3 p, vec3 sunDir )
{
	vec4 C = vec4(c,1.0);
	vec4 P = vec4(p,1.0);
	vec4 F = vec4(0.0,1.0,0.0,-u_fogHeight);
	vec4 V = C-P;
	float a = u_fogDensity;
	float k = dot(F,C) <= 0.0 ? 1.0 : 0.0;

	float sunDot = max( dot( -normalize(V.xyz), sunDir ), 0.0 );
    vec3  fogColor  = mix( u_fogColorAway,
                           u_fogColorTowards,
                           pow(sunDot,4.0) );

	
	vec3 aV = a * V.xyz * 0.5;
	float c1 = k * (dot(F,P) + dot(F,C));
	float c2 = (1.0-2.0*k) * dot(F,P);
	float g = min(c2, 0.0);
	g = -length(aV) * (c1 - g * g / abs(dot(F,V)));
	

	float f = clamp(exp2(-g*g), 0.0, 1.0);

	return mix( fogColor, rgb, f );
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
	float dirtHeight = in_rockData.w;
	float rockHeight = in_rockData.x;

	// Diffuse
	float diffuseScalar = moltenPhase;
	vec3 diffuse = vec3(0.04f);
	diffuse += mappingDataC.y * 0.04;

	float dirtScalar = clamp( dot(vec3(0.0,1.0,0.0), in_rockNormal.xyz), 0.0, 1.0 );
	dirtScalar = smoothstep( 0.5, 1.0, dirtScalar );
	diffuse = mix( diffuse, vec3(0.3,0.2,0.0), smoothstep(0.0, 0.01, dirtHeight) * 0.25 * dirtScalar );
	

	diffuse = max(vec3(0.0f), diffuse-max(0.0f,heat-0.0f)*0.05f);		// Scorch

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

	outColor = ApplyFog( outColor, u_cameraPos, worldPosition, u_lightDir );




	//outColor = vec3(cameraRay.y);

	out_viewPosition = in_viewPosition;
	out_viewNormal = vec4( normalize( in_waterNormal.xyz * mat3(u_viewMatrix) ), 1.0f );
	out_albedo = vec4( 0.0f );
	out_material = vec4( 0.0f, 0.0f, 0.0f, 0.0f );	// roughness, reflectivity, emissive, nowt
	out_forward = vec4(outColor, 1.0f);
}