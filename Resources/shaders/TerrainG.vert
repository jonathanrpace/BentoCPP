#version 330 core


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

// Attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;


// Uniforms
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

uniform vec3 u_rockColorA;
uniform vec3 u_rockColorB;
uniform float u_rockRoughnessA;
uniform float u_rockRoughnessB;
uniform float u_rockFresnelA;
uniform float u_rockFresnelB;
uniform float u_moltenMapOffset;

uniform vec3 u_hotRockColorA;
uniform vec3 u_hotRockColorB;
uniform float u_hotRockRoughnessA;
uniform float u_hotRockRoughnessB;
uniform float u_hotRockFresnelA;
uniform float u_hotRockFresnelB;

uniform vec3 u_moltenColor;
uniform float u_moltenAlphaScalar;
uniform float u_moltenAlphaPower;

uniform vec3 u_dirtColor;

uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_normalModelViewMatrix;
uniform float u_mapHeightOffset;

// Textures
uniform sampler2D s_heightData;
uniform sampler2D s_velocityData;
uniform sampler2D s_miscData;
uniform sampler2D s_normalData;
uniform sampler2D s_diffuseMap;
uniform sampler2D s_moltenMapData;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

// GL
out gl_PerVertex 
{
	vec4 gl_Position;
};

// Varying
out Varying
{
	vec4 out_viewPosition;
	vec4 out_viewNormal;
	vec4 out_albedo;
	vec4 out_material;
	vec4 out_forward;
	vec2 out_uv;
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

void main(void)
{
	// Pluck some values out of the texture data
	vec4 heightDataC = texture(s_heightData, in_uv);
	vec4 velocityDataC = texture(s_velocityData, in_uv);
	vec4 miscDataC = texture(s_miscData, in_uv);
	
	vec3 rockNormal = vec3(0.0);
	float strength = 1.0;
	for ( int i = 1; i < 4; i++ )
	{
		vec4 normalData = textureLod(s_normalData, in_uv, i);
		rockNormal += reconstructNormal(normalData.zw) * strength;
		strength *= 0.5;
	}
	rockNormal = normalize(rockNormal);

	// Common values
	float rockHeight = heightDataC.x;
	float moltenHeight = heightDataC.y;
	float dirtHeight = heightDataC.z;
	float waterHeight = heightDataC.w;
	float heat = miscDataC.x;
	float occlusion = 1.0f - miscDataC.w;
	float moltenMapValue = clamp(miscDataC.y, 0.0, 1.0);
	vec3 viewDir = normalize(u_cameraPos);

	vec4 position = vec4(in_position, 1.0f);
	position.y += rockHeight;
	position.y += moltenHeight;
	position.y += dirtHeight;
	position.y += moltenMapValue * u_mapHeightOffset;

	vec4 viewPosition = position * u_modelViewMatrix;

	// Rock material
	vec3 rockDiffuse = pow( mix( u_rockColorA, u_rockColorB, moltenMapValue ), vec3(2.2) );
	float rockRoughness = mix( u_rockRoughnessA, u_rockRoughnessB, moltenMapValue );
	float rockFresnel = mix( u_rockFresnelA, u_rockFresnelB, moltenMapValue );

	// Hot rock material
	vec3 hotRockDiffuse = pow( mix( u_hotRockColorA, u_hotRockColorB, moltenMapValue ), vec3(2.2) );
	float hotRockRoughness = mix( u_hotRockRoughnessA, u_hotRockRoughnessB, moltenMapValue );
	float hotRockFresnel = mix( u_hotRockFresnelA, u_hotRockFresnelB, moltenMapValue );
	
	// Mix rock and hot rock together
	float hotRockMaterialLerp = min( max( heat-0.1, 0.0 ) / 0.3, 1.0 );

	vec3 diffuse = mix( rockDiffuse, hotRockDiffuse, hotRockMaterialLerp );
	float roughness = mix( rockRoughness, hotRockRoughness, hotRockMaterialLerp );
	float fresnel = mix( rockFresnel, hotRockFresnel, hotRockMaterialLerp );

	// Dirt
	vec3 dirtDiffuse = pow(u_dirtColor, vec3(2.2));
	float dirtAlpha = clamp((dirtHeight / 0.01) - (moltenMapValue * 0.02), 0.0, 1.0);
	diffuse = mix(diffuse, dirtDiffuse, dirtAlpha);

	// Direct light
	float directLight = lightingGGX( rockNormal, viewDir, u_lightDir, roughness, fresnel ) * u_lightIntensity;

	// Ambient light
	float ambientlight = lightingGGX( rockNormal, viewDir, rockNormal, roughness, fresnel ) * u_ambientLightIntensity * occlusion;
	
	// Emissive
	float emissiveAlpha = max( max(heat-0.3, 0.0) * u_moltenAlphaScalar - (moltenMapValue * u_moltenAlphaPower), 0.0 );
	vec3 emissiveColor = pow( mix( u_moltenColor, u_moltenColor * 4.0, emissiveAlpha ), vec3(2.2) );

	// Bing it all together
	vec3 outColor = (diffuse * (directLight + ambientlight));
	outColor *= pow( clamp(1.0 - emissiveAlpha, 0.0, 1.0), 4.0 );
	outColor = mix( outColor, emissiveColor, emissiveAlpha );

	// Fog
	{
		mat4 invViewMatrix = inverse(u_viewMatrix);
		vec3 worldPosition = vec3( invViewMatrix * vec4(viewPosition.xyz,1) );
		vec3 cameraRay = worldPosition-u_cameraPos;
		outColor = ApplyFog( outColor, u_cameraPos, worldPosition, u_lightDir );
	}

	// Output
	{
		out_viewPosition = viewPosition;
		out_viewNormal = vec4( normalize( rockNormal * mat3(u_viewMatrix) ), 1.0f );
		out_albedo = vec4( 0.0f );
		out_material = vec4( 0.0f, 0.0f, 0.0f, 0.0f );	// roughness, reflectivity, emissive, nowt
		out_forward = vec4(outColor, 1.0f);
		out_uv = in_uv;
		gl_Position = u_mvpMatrix * position;
	}
} 