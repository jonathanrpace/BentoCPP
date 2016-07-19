#version 330 core

// Outputs
layout( location = 0 ) out vec4 out_fragColor;

in Varying
{
	vec4 in_color;
	float in_angle;
	float in_life;
	float in_alpha;
	float in_offset;
	float in_heat;
	float in_translucency;
};

uniform sampler2D s_texture;
uniform sampler3D s_texture3D;

uniform vec3 u_moltenColor;


void main(void)
{
	vec2 uv = gl_PointCoord;
	float cosAngle = cos(in_angle);
	float sinAngle = sin(in_angle);

	vec2 rotatedUV = uv;
	rotatedUV -= vec2(0.5);
	rotatedUV = vec2( rotatedUV.x * cosAngle - rotatedUV.y * sinAngle,
					  rotatedUV.x * sinAngle + rotatedUV.y * cosAngle );
	rotatedUV += vec2(0.5);

	//vec4 textureSample = textureLod(s_texture, rotatedUV, 1.0 + (1.0-in_life) * 4.0);

	float t = 1-in_life;

	vec4 textureSample3D = texture(s_texture3D, vec3(gl_PointCoord, pow(t, 0.9)));

	

	float alpha = textureSample3D.a * pow( in_life, 0.7 );// clamp((textureSample3D.a - 0.7) + t * 0.7, 0.0, 1.0) * in_life;

	alpha = pow( alpha, mix(3.0, 1.0, t) );


	//float alphaMult = textureSample3D.a*pow(in_life, 0.75);

	//float alpha = alphaMult;//clamp( mix(alphaOffset, alphaMult, in_life), 0.0, 1.0 );


	//alpha *= in_alpha;	// Spawn alpha


	//max( (textureSample3D.x*pow(in_life, 0.75)) * 1.0, 0.0 );

	//alpha += textureSample3D.x * in_life;

	/*
	float density = textureSample.b;
	float ao = textureSample.a;
	float lifeAlpha = in_color.w;

	float alpha = density * lifeAlpha * in_alpha;

	if ( alpha <= 0.01 )
		discard;

	vec3 normal = vec3(textureSample.rg*vec2(2.0) - vec2(1.0), 0.0);
	normal.z = 1.0 - length(normal);
	normal.xy = vec2( normal.x * cosAngle - normal.y * sinAngle,
					  normal.x * sinAngle + normal.y * cosAngle );
	

	float lightFromSky = clamp( dot( normal, vec3(0.0, 1.0, 0.0) ), 0.0, 1.0 ) * 0.2;
	float ambientLight = 0.2 * ao;

	float emissiveAlpha = max(in_heat-0.3, 0.0);
	vec3 emissiveColor = pow( mix( u_moltenColor, u_moltenColor * 4.0, emissiveAlpha ), vec3(2.2) );

	vec3 lightFromHeat = emissiveColor * clamp( dot( normal, vec3(0.0, -1.0, 0.0) ), 0.0, 1.0 ) * emissiveAlpha;


	vec3 color = vec3( lightFromSky + lightFromHeat + ambientLight );

	float translucentScalar = max( (1.0-density) * 2.0, 0.0 );
	color += max(0.0, in_translucency * translucentScalar );

	out_fragColor = vec4( color, alpha );
	*/

	out_fragColor = vec4(pow(textureSample3D.ggg, vec3(1.5)), 0.0);

} 