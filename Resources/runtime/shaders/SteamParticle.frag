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

uniform vec3 u_moltenColor;


void main(void)
{
	vec2 uv = gl_PointCoord;
	float cosAngle = cos(in_angle);
	float sinAngle = sin(in_angle);

	vec4 textureSampleA = texture(s_texture, uv);

	vec4 textureSampleA2 = texture(s_texture, uv - vec2(0.0, in_offset));

	vec2 rotatedUV = uv;
	rotatedUV.x -= textureSampleA2.b * (1.0-in_life) * 0.1;
	rotatedUV.y -= textureSampleA2.r * (1.0-in_life) * 0.2;
	rotatedUV.y += in_offset;

	vec4 textureSampleB = texture(s_texture, rotatedUV);

	float alpha = textureSampleA.b * textureSampleB.r * in_alpha * 1.5;
	alpha -= (1.0-in_alpha) * 0.4;
	alpha = clamp(alpha, 0.0,1.0);

	if ( alpha <= 0.02 )
		discard;


	float emissiveAlpha = max(in_heat, 0.0);
	vec3 emissiveColor = pow( mix( u_moltenColor, u_moltenColor * 4.0, emissiveAlpha ), vec3(2.2) );

	vec3 lightFromHeat = emissiveColor * textureSampleA.r * pow(textureSampleA2.r, 2.0) * emissiveAlpha * in_life * 2.0;


	float t = 1.0-in_life;

	vec4 sparksTextureSample = pow( texture(s_texture, uv + vec2(textureSampleA.b * 0.1 * (in_life), pow(in_life, 10.0) * -4.0) ), vec4(2.2) );
	lightFromHeat += sparksTextureSample.g * emissiveColor * in_alpha * pow( textureSampleA.b * textureSampleA2.r * textureSampleB.b, 2.0 ) * 50.0;

	vec3 color = vec3( lightFromHeat );


	/*
	float density = textureSample.b;
	float ao = textureSample.a;
	float lifeAlpha = in_color.w;

	float alpha = density * lifeAlpha * in_alpha;

	
	vec3 normal = vec3(textureSample.rg*vec2(2.0) - vec2(1.0), 0.0);
	normal.z = 1.0 - length(normal);
	normal.xy = vec2( normal.x * cosAngle - normal.y * sinAngle,
					  normal.x * sinAngle + normal.y * cosAngle );
	

	float lightFromSky = clamp( dot( normal, vec3(0.0, 1.0, 0.0) ), 0.0, 1.0 ) * 0.2;
	float ambientLight = 0.2 * ao;

	

	float translucentScalar = max( (1.0-density) * 2.0, 0.0 );
	color += max(0.0, in_translucency * translucentScalar );

	out_fragColor = vec4( color, alpha );
	*/

	out_fragColor = vec4(color, alpha );//vec4(pow(textureSample3D.ggg, vec3(1.5)), 0.0);

} 