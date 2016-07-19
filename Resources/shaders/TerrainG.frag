#version 430 core


////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Varying
in Varying
{
	vec4 in_viewPosition;
	vec4 in_viewNormal;
	vec4 in_albedo;
	vec4 in_material;
	vec4 in_forward;
	vec2 in_uv;
	float in_dirtAlpha;
};

// Uniforms
uniform vec2 u_mouseScreenPos;
uniform ivec2 u_windowSize;

uniform sampler2D s_dirtDiffuse;
uniform sampler2D s_rockDiffuse;
uniform sampler2D s_dirtNormal;
uniform float u_dirtTextureRepeat;

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

	vec4 dirtDiffuse = pow( texture( s_dirtDiffuse, in_uv * u_dirtTextureRepeat ), vec4(2.2) );
	vec4 rockDiffuse = pow( texture( s_rockDiffuse, in_uv * u_dirtTextureRepeat ), vec4(2.2) );
	//vec4 dirtNormal = texture( s_dirtNormal, in_uv * u_dirtTextureRepeat );

	vec4 forward = in_forward;

	//forward.rgb *= rockDiffuse.rgb;

	forward = mix( forward, dirtDiffuse, in_dirtAlpha );

	out_viewPosition = in_viewPosition;
	out_viewNormal = in_viewNormal;
	out_albedo = in_albedo;
	out_material = in_material;
	out_forward = forward;
}
