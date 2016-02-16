#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Varying
in Varying
{
	vec2 in_uv;
	vec4 in_rockData;
	vec4 in_waterData;
	vec4 in_waterNormal;
	vec4 in_rockNormal;
	vec4 in_waterFluxData;
	vec4 in_viewPosition;
	vec4 in_worldPosition;
	vec4 in_screenPosition;
	vec2 in_foamUVA;
	vec2 in_foamUVB;
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
uniform vec4 u_phase;

// Material
uniform float u_specularPower;
uniform float u_fresnelPower = 1.5f;
uniform vec3 u_waterColor;
uniform vec3 u_waterTranslucentColor;
uniform float u_indexOfRefraction;

// Samplers
uniform sampler2D s_rockData;
uniform sampler2D s_waterData;
uniform sampler2D s_waterFluxData;
uniform sampler2D s_mappingData;
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
	float occlusion = 1.0f - in_rockNormal.w;
	vec3 eye = normalize( in_viewPosition.xyz * mat3(u_viewMatrix) );
	vec4 rockDataSample = texture2D( s_rockData, in_uv );
	vec4 waterDataSample = texture2D( s_waterData, in_uv );
	float fresnel = 1.0f - clamp(dot(in_waterNormal.xyz,-eye), 0.0f, 1.0f);
	fresnel = pow( fresnel, u_fresnelPower );

	vec4 mappingDataC = texture2D( s_mappingData, in_uv );

	vec4 targetViewPosition = texelFetch(s_positionBuffer, ivec2(gl_FragCoord.xy));
	if ( targetViewPosition.z == 0.0f ) targetViewPosition.z = in_viewPosition.z + 50.0f;
	float viewDepth = abs( in_viewPosition.z - targetViewPosition.z );
	viewDepth = min(viewDepth, 0.5f);
	float waterAlpha = min(1.0f, viewDepth / 0.01f );
	waterAlpha = pow( waterAlpha, 0.8f );

	////////////////////////////////////////////////////////////////
	// Refraction
	////////////////////////////////////////////////////////////////
	vec3 outColor = vec3(0.0f);
	{
		// We do this first as we need to know what pixel in the frame-buffer we're blending with
		vec4 screenPosition = in_screenPosition;
		screenPosition.xyz /= screenPosition.w;
		screenPosition.xy += 1.0f;
		screenPosition.xy *= 0.5f;
		vec3 refractVec = -normalize( refract(eye, -in_waterNormal.xyz, u_indexOfRefraction) );
		
		vec4 samplePos = vec4( in_worldPosition );
		samplePos.xyz += refractVec * viewDepth;
		samplePos *= u_mvpMatrix;
		samplePos.xyz /= samplePos.w;
		samplePos.xy += 1.0f;
		samplePos.xy *= 0.5f;

		vec2 dimensions = vec2(textureSize( s_output, 0 ));

		outColor = texture2DRect(s_output, samplePos.xy * dimensions).xyz;
	}

	////////////////////////////////////////////////////////////////
	// Filter
	////////////////////////////////////////////////////////////////
	{
		vec3 waterColor = diffuse(in_waterNormal.xyz, u_lightDir, 1.0f) * u_waterColor * u_lightIntensity;
		waterColor = pow(waterColor, vec3(0.75f));
		waterColor *= waterAlpha;
		
		// Filter color behind water. The deeper the water, the more filter applied.
		outColor *= mix( vec3(1.0f), waterColor, waterAlpha );
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
		float translucentDot = pow( max( dot( translucentVec, u_lightDir ), 0.0f ), 1.5f ) * max( 0.01f, pow( max( dot(refractedLightDir, eye), 0.0f ), 1.5f ) );

		outColor += u_waterTranslucentColor * pow(translucentDot,0.5f) * translucency * u_lightIntensity * waterAlpha;
	}
	
	////////////////////////////////////////////////////////////////
	// Reflections
	////////////////////////////////////////////////////////////////
	{
		// Specular
		vec3 waterSpecular = vec3( specular( in_waterNormal.xyz, u_lightDir, eye, u_specularPower ) * u_lightIntensity );
		outColor += waterSpecular * fresnel * waterAlpha;

		// Sky
		float skyReflect = clamp(dot(in_waterNormal.xyz,-eye), 0.0f, 1.0f);
		skyReflect = smoothstep(0.45f, 0.55f, fresnel);
		outColor += skyReflect * vec3(pow(fresnel, 1.5f)) * 0.15f * waterAlpha;
	}
	
	////////////////////////////////////////////////////////////////
	// Foam
	////////////////////////////////////////////////////////////////
	{
		vec2 frothUV = in_uv + in_waterNormal.xz * 0.0f;
		float foamAlpha = texture2D( s_waterData, frothUV ).w;

		float foamTextureA = texture2D( s_diffuseMap, in_foamUVA * 1.5).z;
		float foamTextureB = texture2D( s_diffuseMap, in_foamUVB * 1.5).z;
		float phaseA = u_phase.y;
		float phaseB = u_phase.x;

		float foamTexture = foamTextureA*phaseB + foamTextureB*phaseA;

		foamAlpha *= foamTexture;//(foamTexture - (1.0-foamAlpha)) / mix(foamAlpha, 1.0, 0.5);



		//foamAlpha = mix(foamAlpha*foamTexture, foamAlpha, pow(foamAlpha,2.0));

		//foamAlpha *= foamTexture;
		foamAlpha = clamp(foamAlpha, 0, 1);

		// Light the foam
		vec3 foamDiffuse = vec3( mix(0.0, 1.0, pow( foamAlpha, 2.0)) );
		float foamDiffuseDot = clamp( dot( in_waterNormal.xyz, u_lightDir ) * 0.9f + 0.1f, 0.0f, 1.0f );
		foamDiffuse *= foamDiffuseDot * u_lightIntensity + u_ambientLightIntensity;

		float foamSpecular = 0.0f;//specular( in_waterNormal.xyz, u_lightDir, eye, 1.0f ) * u_lightIntensity * 0.25f;

		// Blend foam on top of current output
		//outColor = mix( outColor, foamDiffuse + foamSpecular, foamAlpha * waterAlpha );
	}

	
	//outColor = vec3(mappingDataC.w);
	out_forwad = vec4( outColor, 0.0f );
}