#version 330 core

// From VS
in Varying
{
	vec2 in_uv;
};

layout( location = 0 ) out vec4 out_heightData;

uniform sampler2D s_velocityTexture;
uniform sampler2D s_heightData;

uniform float u_dt;
uniform float u_dissipation;

void main()
{
	vec4 heightDataOld = texelFetch( s_heightData, ivec2(gl_FragCoord.xy), 0 );
	
    vec4 velocitySample = texture(s_velocityTexture, in_uv);
	vec2 velocityA = velocitySample.xy;
	vec2 velocityB = velocitySample.zw;
	
	vec4 heightDataNewA = u_dissipation * texture(s_heightData, in_uv - velocityA * u_dt);
	vec4 heightDataNewB = u_dissipation * texture(s_heightData, in_uv - velocityB * u_dt);
	
    out_heightData = vec4( heightDataOld.x, heightDataNewA.y, heightDataOld.z, heightDataNewB.w );
}