#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Varying
in Varying
{
	vec2 in_uv;
	vec4 in_heightData;
	vec4 in_velocityData;
	vec4 in_miscData;
	vec4 in_normalData;
	vec4 in_viewPosition;
	vec4 in_worldPosition;
	vec4 in_screenPosition;
};

// Uniforms ////////////////////////////////////////////////////

// Matrices
uniform mat4 u_mvpMatrix;
uniform mat4 u_viewMatrix;

// Lighting
uniform vec3 u_lightDir;
uniform float u_lightIntensity;
uniform float u_ambientLightIntensity;

// Dynamics
uniform float u_phase;

// Material
uniform float u_specularPower;
uniform float u_fresnelPower = 4.0f;
uniform vec3 u_waterColor;
uniform vec3 u_waterTranslucentColor;
uniform float u_indexOfRefraction;

// Samplers
uniform sampler2D s_diffuseMap;
uniform sampler2DRect s_output;
uniform sampler2DRect s_positionBuffer;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_forwad;

////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////

vec3 reconstructNormal( vec2 normal2 )
{
	float len = length(normal2);
	vec3 normal3 = vec3(normal2.x, 1.0-len, normal2.y);
	return normalize(normal3);
}


// lighting
float diffuse(vec3 n, vec3 l, float p)
{
    return pow(dot(n,l) * 0.5 + 1.0, p);
}
float specular(vec3 n, vec3 l, vec3 e, float s)
{    
    float nrm = (s + 8.0) / (3.1415 * 8.0);
    return pow(max(dot(reflect(e,n),l),0.0),s) * nrm;
}

void main(void)
{
	// Grab a load of commonly used values
	float occlusion = 1.0f - in_miscData.w;
	vec3 eye = -normalize( in_viewPosition.xyz * mat3(u_viewMatrix) );
	
	vec3 waterNormal = reconstructNormal(in_normalData.xy);

	float fresnel = 1.0f - clamp(dot(waterNormal,eye), 0.0f, 1.0f);
	fresnel = pow( fresnel, u_fresnelPower );
	fresnel = mix( 0.02, 1.0, fresnel );

	vec4 targetViewPosition = texelFetch(s_positionBuffer, ivec2(gl_FragCoord.xy));
	if ( targetViewPosition.z == 0.0 ) targetViewPosition.z = in_viewPosition.z + 50.0;
	float viewDepth = abs( in_viewPosition.z - targetViewPosition.z );
	viewDepth = clamp(viewDepth, 0.0, 0.05);

	float waterAlpha = min( in_heightData.w / 0.0001, 1.0 );

	////////////////////////////////////////////////////////////////
	// Refraction
	////////////////////////////////////////////////////////////////
	vec3 outColor = vec3(0.0f);
	{
		// We do this first as we need to know what pixel in the frame-buffer we're blending with
		vec3 refractVec = refract(-eye, waterNormal, u_indexOfRefraction);
		
		vec4 samplePos = vec4( in_worldPosition );
		//samplePos.xyz += refractVec * viewDepth;
		samplePos *= u_mvpMatrix;
		samplePos.xyz /= samplePos.w;
		samplePos.xy += 1.0;
		samplePos.xy *= 0.5;

		vec2 dimensions = vec2(textureSize( s_output, 0 ));

		outColor = texture2DRect(s_output, samplePos.xy * dimensions).xyz;
	}
	/*
	////////////////////////////////////////////////////////////////
	// Filter
	////////////////////////////////////////////////////////////////
	{
		vec3 waterDiffuse = vec3( diffuse(in_waterNormal.xyz, u_lightDir, 1.5f) * u_lightIntensity );
		waterDiffuse += u_ambientLightIntensity;
		waterDiffuse *= u_waterColor;

		//waterColor = pow(waterColor, vec3(0.75f));
		//waterColor *= waterAlpha;
		
		// Filter color behind water. The deeper the water, the more filter applied.
		outColor *= mix( vec3(1.0f), waterDiffuse, waterAlpha );
	}

	////////////////////////////////////////////////////////////////
	// Translucency
	////////////////////////////////////////////////////////////////
	{
		float waterHeight = rockDataSample.x + rockDataSample.y + rockDataSample.a + waterDataSample.x + waterDataSample.y;
		float translucency = 0.0f;
		float strength = 0.5f;
		for ( int i = 0; i < 8; i++ )
		{ 
			vec4 rockDataSampleMipped = texture2D( s_rockData, in_uv, 1+i );
			vec4 waterDataSampleMipped = texture2D( s_waterData, in_uv, 1+i );
			float waterHeightMipped = rockDataSampleMipped.x + rockDataSampleMipped.y + rockDataSampleMipped.a + waterDataSampleMipped.x + waterDataSampleMipped.y;

			translucency += max( waterHeight - waterHeightMipped, 0.0f ) * strength;
			strength *= 1.5f;
		}
		translucency = min( translucency, 1.0f );
		
		// Reflect the water normal about the up vector
		// From observation, this gives us a good enough approximation of the 'back face' normal. 
		vec3 translucentVec = reflect(-in_waterNormal.xyz, vec3(0.0f,1.0f,0.0f));

		// We now take this vector and refract it backwards through the wave.
		vec3 refractedLightDir = -refract(-u_lightDir, translucentVec, u_indexOfRefraction);
		float translucentDot = pow( max( dot( translucentVec, u_lightDir ), 0.0f ), 2.5f ) * max( 0.01f, pow( max( dot(refractedLightDir, -eye), 0.0f ), 2.5f ) );

		outColor += u_waterTranslucentColor * pow(translucentDot,0.5f) * translucency * u_lightIntensity * waterAlpha;
	}
	
	////////////////////////////////////////////////////////////////
	// Reflections
	////////////////////////////////////////////////////////////////
	{
		// Specular
		vec3 waterSpecular = vec3( specular( in_waterNormal.xyz, u_lightDir, -eye, u_specularPower ) * u_lightIntensity );
		outColor += waterSpecular * fresnel * waterAlpha;

		// Sky
		float skyReflect = clamp( (dot(reflect(-eye, in_waterNormal.xyz), vec3(0.0,1.0,0.0)) + 1.0) * 0.5, 0.0, 1.0 );
		outColor += skyReflect * waterAlpha * fresnel;
	}
	*/

	/*
	////////////////////////////////////////////////////////////////
	// Foam
	////////////////////////////////////////////////////////////////
	{
		vec4 foamSample = texture2D( s_foamData, in_uv );

		vec3 foamDiffuse = vec3( diffuse(in_waterNormal.xyz, u_lightDir, 4.0f) * u_lightIntensity );
		foamDiffuse += u_ambientLightIntensity;
		foamDiffuse *= vec3(0.7);	// Albedo

		outColor = mix(outColor, foamDiffuse, foamSample.w * 0.75 * waterAlpha);
	} 
	*/

	//vec2 velocityColor = (clamp(in_waterData.yz, vec2(-1.0), vec2(1.0))+1.0) * 0.5;
	

	float speed = length(in_velocityData.zw);
	vec3 velocityColor = mix( vec3( 0.0, 0.0, 0.2 ), vec3( 0.0, 0.1, 0.6 ), speed );
	
	float dissolvedDirtProp = 0.0;//min( in_miscData.z / 0.01, 1.0 );
	vec3 waterColor = mix( velocityColor, vec3(1.0, 0.0, 0.0), dissolvedDirtProp );

	waterColor = pow(waterColor, vec3(2.2)); // Gamma correct


	outColor = mix(outColor, waterColor, waterAlpha*1.0);



	out_forwad = vec4( outColor, 0.0f );
}