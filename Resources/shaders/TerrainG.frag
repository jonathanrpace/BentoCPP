#version 430 core

////////////////////////////////////////////////////////////////
// Lighting lib
////////////////////////////////////////////////////////////////

float lightingVis(float dotNV, float k)
{
	return 1.0f/(dotNV*(1.0f-k)+k);
}

float lightingAngularDot( vec3 A, vec3 B, in float angularSize )
{
	float dotValue = clamp( dot( A, B ), 0.0f, 1.0f );
	float pi = 3.14159f;

	float angle = 1.0f - acos(dotValue) * (2.0f / pi);
	angle = min( angle, angularSize );
	angle *= ( 1 / angularSize );

	float ret = sin( angle * pi * 0.5f );
	
	return ret;
}

float lightingFresnel( float dotNV, float F0 )
{
	float dotNVPow = pow(1.0f-dotNV,2);
	float F = F0 + (1.0-F0)*(dotNVPow);
	return F;
}

float lightingGGXAngular( vec3 N, vec3 V, vec3 L, float roughness, float F0, float angularSize )
{
	float alpha = roughness*roughness;

	vec3 H = normalize(V+L);

	float dotNL = lightingAngularDot(N,L,angularSize);
	float dotNV = clamp(dot(N,V), 0.0f, 1.0f);
	float dotNH = lightingAngularDot(N,H,angularSize);
	float dotLH = clamp(dot(L,H), 0.0f, 1.0f);

	// D
	float alphaSqr = alpha*alpha;
	float pi = 3.14159f;
	float denom = dotNH * dotNH *(alphaSqr-1.0) + 1.0f;
	float D = alphaSqr/(pi * denom * denom);

	// F
	float F = lightingFresnel(dotNV, F0);

	// V
	float k = alpha/2.0f;
	float vis = lightingVis(dotNL,k)*lightingVis(dotNV,k);

	return min( 10.0f, dotNL * D * F * vis );
}

float lightingGGX( vec3 N, vec3 V, vec3 L, float roughness, float F0 )
{
	float alpha = roughness*roughness;

	vec3 H = normalize(V+L);

	float dotNL = clamp(dot(N,L), 0.0f, 1.0f);
	float dotNV = clamp(dot(N,V), 0.0f, 1.0f);
	float dotNH = clamp(dot(N,H), 0.0f, 1.0f);
	float dotLH = clamp(dot(L,H), 0.0f, 1.0f);

	// D
	float alphaSqr = alpha*alpha;
	float pi = 3.14159f;
	float denom = dotNH * dotNH *(alphaSqr-1.0) + 1.0f;
	float D = alphaSqr/(pi * denom * denom);

	// F
	float F = lightingFresnel(dotNV, F0);

	// V
	float k = alpha/2.0f;
	float vis = lightingVis(dotNL,k)*lightingVis(dotNV,k);

	return min( 10.0f, dotNL * D * F * vis );
}

////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Varying
in Varying
{
	vec2 in_uv;
	vec3 in_viewNormal;
	vec4 in_viewPosition;
	vec4 in_heightData;
	vec4 in_velocityData;
	vec4 in_miscData;
	vec4 in_normalData;
	vec4 in_moltenMapData;
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
uniform sampler2D s_smudgeData;
uniform sampler2D s_moltenMapData;


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

vec3 reconstructNormal( vec2 normal2 )
{
	float len = length(normal2);
	vec3 normal3 = vec3(normal2.x, (1.0-len), normal2.y);
	return normalize(normal3);
}

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

	// Common values
	float heat = in_miscData.x;
	float occlusion = 1.0f - in_miscData.w;
	float rockHeight = in_heightData.x;
	float dirtHeight = in_heightData.z;
	vec3 rockNormal = reconstructNormal(in_normalData.zw);
	vec4 smudgeData = texture(s_smudgeData, in_uv);
	vec2 smudgeUV = smudgeData.xy;
	vec4 moltenMapData = texture( s_moltenMapData, in_uv - smudgeUV );
	float moltenMapValue = moltenMapData.x;
	vec3 viewDir = normalize(u_cameraPos);

	// Rock
	vec3 diffuse = vec3(0.0);
	float roughness = 0.5;
	float fresnel = 1.0;


	vec3 rockDiffuse = mix( vec3(0.18f), vec3(0.2f), moltenMapValue );
	const vec3 dirtDiffuse = vec3(0.2,0.15,0.1);

	diffuse = mix( rockDiffuse, dirtDiffuse, smoothstep(0.0, 0.01, dirtHeight) );
	
	float scorchAmount = min( max(heat-0.3, 0.0) / 0.2, 1.0 );
	diffuse -= scorchAmount * diffuse * 0.9;

	diffuse = pow(diffuse, vec3(2.2));	// Gamma correct

	roughness = 0.9;//mix( 1.0, 0.0, clamp(heat, 0.0, 1.0) );

	// Direct light
	//vec3 directLight = vec3( max( dot(rockNormal, u_lightDir), 0.0f ) * u_lightIntensity );
	float directLight = lightingGGX( rockNormal, viewDir, u_lightDir, roughness, fresnel ) * u_lightIntensity;

	// Ambient light
	//vec3 ambientlight = vec3( u_ambientLightIntensity * occlusion );
	float ambientlight = lightingGGX( rockNormal, viewDir, rockNormal, roughness, fresnel ) * u_ambientLightIntensity * occlusion;
	

	// Emissive
	vec3 emissive = vec3(0.0f);
	float heatForColor0 = min(1.0f,heat);
	float heatColor0 = max(0.0f, heatForColor0 - moltenMapValue * 0.5f );
	heatColor0 = pow(heatColor0, 2.0f);

	float heatForColor1 = heat * 0.5f;
	float heatColor1 = max(0.0f, heatForColor1 - moltenMapValue * 0.25f);
	heatColor1 = min(1.0f,heatColor1);
	heatColor1 = pow(heatColor1, 2.0f);

	heatColor0 += heatColor1;

	emissive.x = heatColor0;
	emissive.y = heatColor1;

	emissive = pow(emissive, vec3(2.2));


	// Bing it all together
	vec3 outColor = (diffuse * (directLight + ambientlight)) + emissive;

	mat4 invViewMatrix = inverse(u_viewMatrix);
	vec3 worldPosition = vec3( invViewMatrix * vec4(in_viewPosition.xyz,1) );
	vec3 cameraRay = worldPosition-u_cameraPos;

	outColor = ApplyFog( outColor, u_cameraPos, worldPosition, u_lightDir );


	//outColor = vec3( 0.0, 0.0, dirtHeight / 0.2 );
	//outColor += vec3(in_waterData.yz*20.0, 0.0);
	//outColor = vec3(cameraRay.y);

	out_viewPosition = in_viewPosition;
	out_viewNormal = vec4( normalize( rockNormal * mat3(u_viewMatrix) ), 1.0f );
	out_albedo = vec4( 0.0f );
	out_material = vec4( 0.0f, 0.0f, 0.0f, 0.0f );	// roughness, reflectivity, emissive, nowt
	out_forward = vec4(outColor, 1.0f);

	
	//smudgeData.z = 0.1;
	//smudgeData.xy += 1.0;
	//smudgeData.xy *= 0.5;
	//out_forward = smudgeData;
}
