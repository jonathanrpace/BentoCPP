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

	vec2 rotatedUV = uv;
	rotatedUV -= vec2(0.5);
	rotatedUV = vec2( rotatedUV.x * cosAngle - rotatedUV.y * sinAngle,
					  rotatedUV.x * sinAngle + rotatedUV.y * cosAngle );
	rotatedUV += vec2(0.5);

	vec4 textureSample = texture(s_texture, rotatedUV);

	float density = textureSample.b;
	float ao = textureSample.a;
	float lifeAlpha = in_color.w;

	float alpha = density * lifeAlpha * in_alpha;

	if ( alpha <= 0.0 )
		discard;

	vec3 normal = vec3(textureSample.r, textureSample.g, 0.0);
	normal.z = 1.0 - length(normal);
	normal.xy = vec2( normal.x * cosAngle - normal.y * sinAngle,
					  normal.x * sinAngle + normal.y * cosAngle );


	float lightFromSky = clamp( dot( normal, vec3(0.0, 1.0, 0.0) ), 0.0, 1.0 ) * 0.2;
	float ambientLight = 0.2 * ao;

	float emissiveAlpha = max(in_heat-0.3, 0.0);
	vec3 emissiveColor = pow( mix( u_moltenColor, u_moltenColor * 4.0, emissiveAlpha ), vec3(2.2) );

	vec3 lightFromHeat = emissiveColor * clamp( dot( normal, vec3(0.0, -1.0, 0.0) ), 0.0, 1.0 ) * emissiveAlpha;


	vec3 color = vec3( lightFromSky + lightFromHeat + ambientLight );

	color += max(0.0, in_translucency - density);

	out_fragColor = vec4( color, alpha );


} 