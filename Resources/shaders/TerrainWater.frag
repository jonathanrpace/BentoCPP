#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Varying
in Varying
{
	vec3 in_normal;
	vec4 in_viewPosition;
	vec4 in_worldPosition;
	vec4 in_diffuse;
	vec3 in_reflections;
	float in_alpha;
};

// Uniforms ////////////////////////////////////////////////////

// Matrices
uniform mat4 u_mvpMatrix;
uniform mat4 u_viewMatrix;

// Material
uniform vec3 u_waterColor;
uniform float u_indexOfRefraction;
uniform float u_waterDepthToDiffuse;
uniform float u_dissolvedDirtDepthToDiffuse;

// Samplers
uniform sampler2DRect s_output;
uniform sampler2DRect s_positionBuffer;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_forwad;



////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////

void main(void)
{
	vec3 eye = -normalize( in_viewPosition.xyz * mat3(u_viewMatrix) );
	vec3 normal = normalize(in_normal);

	vec3 outColor = vec3(0.0f);

	vec4 targetViewPosition = texelFetch(s_positionBuffer, ivec2(gl_FragCoord.xy));
	vec4 outputSample = texelFetch(s_output, ivec2(gl_FragCoord.xy));
	//if ( targetViewPosition.z == 0.0 ) targetViewPosition.z = in_viewPosition.z + 50.0;

	float viewDepth = abs( in_viewPosition.z - targetViewPosition.z );

	////////////////////////////////////////////////////////////////
	// Refraction
	////////////////////////////////////////////////////////////////
	{
		// We do this first as we need to know what pixel in the frame-buffer we're blending with
		vec3 refractVec = refract(-eye, normal, u_indexOfRefraction);
		
		vec4 samplePos = vec4( in_worldPosition );
		samplePos.xyz += refractVec;
		samplePos *= u_mvpMatrix;
		samplePos.xyz /= samplePos.w;
		samplePos.xy += 1.0;
		samplePos.xy *= 0.5;

		vec2 dimensions = vec2(textureSize( s_output, 0 ));
		outColor = mix( outputSample.rgb, texture2DRect(s_output, samplePos.xy * dimensions).rgb, in_alpha );
	}
	
	// Filter color behind water. The deeper the water, the more filter applied.
	float waterFilterAlpha = clamp( viewDepth / u_waterDepthToDiffuse, 0.0, 1.0 );
	outColor *= mix( vec3(1.0f), u_waterColor, waterFilterAlpha );

	// Alpha blend diffuse
	float waterDiffuseAlpha = clamp( viewDepth / u_waterDepthToDiffuse, 0.0, 1.0 );
	float dirtDiffuseAlpha = clamp( viewDepth / u_dissolvedDirtDepthToDiffuse, 0.0, 1.0 ) * in_diffuse.a;	// in_diffuse.a == dissolvedDirtDensity
	float diffuseAlpha = min( dirtDiffuseAlpha, 1.0 );
	outColor = mix( outColor, in_diffuse.rgb, diffuseAlpha * in_alpha );

	// Add reflections
	outColor += in_reflections * in_alpha;

	/*
	////////////////////////////////////////////////////////////////
	// Debug
	////////////////////////////////////////////////////////////////
	{
		float speed = length(in_velocityData.zw);
		vec3 velocityColor = mix( vec3( 0.0, 0.0, 0.2 ), vec3( 0.0, 0.1, 0.6 ), speed );
	
		float dissolvedDirtProp = min( in_miscData.z / 0.01, 1.0 );
		vec3 waterColor = mix( velocityColor, vec3(1.0, 0.0, 0.0), dissolvedDirtProp );

		waterColor = pow(waterColor, vec3(2.2)); // Gamma correct

		outColor = mix(outColor, waterColor, in_alpha*1.0);
	}
	*/

	out_forwad = vec4( outColor, 0.0f );
}