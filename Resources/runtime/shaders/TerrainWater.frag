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
	float in_reflectAlpha;
	float in_dissolvedDirtAlpha;
	vec3 in_eyeVec;
	float in_specularOcclusion;
	vec2 in_waterVelocity;
	vec2 in_uv;
	float in_foamStrength;
};

// Uniforms ////////////////////////////////////////////////////

// Matrices
uniform mat4 u_mvpMatrix;
uniform mat4 u_viewMatrix;

// Material
uniform vec3 u_waterColor;
uniform vec3 u_dirtColor;
uniform float u_indexOfRefraction;
uniform float u_depthToFilter;
uniform float u_depthToDiffuse;
uniform float u_fresnelPower = 4.0f;
uniform float u_specularPower;

// Flow map
uniform float u_phaseA;
uniform float u_phaseB;
uniform float u_phaseAlpha;
uniform float u_waterFlowOffset;
uniform float u_waterFlowRepeat;

// Foam
uniform float u_foamRepeat;
uniform float u_foamDistortStrength;
uniform float u_foamAlphaStrength;

uniform float u_waveFrequency = 2.0;
uniform float u_waveAmplitude = 0.03;
uniform float u_waveChoppy = 1.0;
uniform int u_waveLevels = 5;
uniform float u_waveTime = 0.0;
mat2 octave_m = mat2(1.6,1.2,-1.2,1.6);

// Lighting
uniform vec3 u_lightDir;
uniform float u_lightDistance;
uniform float u_lightIntensity;
uniform float u_ambientLightIntensity;

// Samplers
uniform sampler2DRect s_output;
uniform sampler2DRect s_positionBuffer;
uniform sampler2D s_foamMap;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_forwad;

////////////////////////////////////////////////////////////////
// STD Lib Functions
////////////////////////////////////////////////////////////////
float lightingGGX( vec3 N, vec3 V, vec3 L, float roughness, float F0 );

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////
float hash( vec2 p ) 
{
	float h = dot(p,vec2(127.1,311.7));	
    return fract(sin(h)*43758.5453123);
}

float noise( in vec2 p ) 
{
    vec2 i = floor( p );
    vec2 f = fract( p );	
	vec2 u = f*f*(3.0-2.0*f);
    return -1.0+2.0*mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}

float waveNoiseOctave(vec2 uv, float choppy) 
{
    uv += noise(uv);        
    vec2 wv = 1.0-abs(sin(uv));
    vec2 swv = abs(cos(uv));    
    wv = mix(wv,swv,wv);
    return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
}

float waveNoise(vec3 p, int iter, float ampScalar, float _choppy) 
{
    float freq = u_waveFrequency;
    float amp = u_waveAmplitude;
    float choppy = _choppy;
    vec2 uv = p.xz * vec2(1.0, 0.75);
    
    float d = 0.0;
	float h = 0.0;    
    for(int i = 0; i < iter; i++) 
	{        
    	d = waveNoiseOctave((uv+u_waveTime)*freq,choppy);
    	d += waveNoiseOctave((uv-u_waveTime)*freq,choppy);
        h += d * amp;        
    	uv *= octave_m; 
		freq *= 1.9; 
		amp *= ampScalar;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

vec3 waveNormal(vec3 p, float eps, int iter, float ampScalar, float choppy)
{
    vec3 n;
    n.y = waveNoise(p, iter, ampScalar, choppy);    
    n.x = waveNoise(vec3(p.x+eps,p.y,p.z), iter, ampScalar, choppy) - n.y;
    n.z = waveNoise(vec3(p.x,p.y,p.z+eps), iter, ampScalar, choppy) - n.y;
    n.y = eps;
    return normalize(n);
}

vec3 getSkyColor(vec3 e) 
{
    e.y = max(e.y,0.0);
    vec3 ret;
    ret.x = pow(1.0-e.y,2.0);
    ret.y = 0.1 +(1.0-e.y) * 0.9;
    ret.z = 0.3+(1.0-e.y)*0.7;
    return ret;
}

vec2 rotateBy( vec2 _pt, float _angle )
{
	float cosValue = cos(_angle);
	float sinValue = sin(_angle);
	return vec2( _pt.x * cosValue - _pt.y * sinValue, _pt.x * sinValue + _pt.y * cosValue );
}

////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////

void main(void)
{
	vec3 eye = normalize(in_eyeVec);
	vec3 normal = normalize(in_normal);
	vec3 tangent = cross(normal, vec3(0.0,0.0,1.0));
	vec3 bitangent = -cross(normal, tangent);
	vec3 lightDir = normalize(u_lightDir * u_lightDistance - in_worldPosition.xyz);
	float diffuseLighting = lightingGGX(normal, eye, lightDir, 1.0, 0.0) * u_lightIntensity + u_ambientLightIntensity;
	
	// Peturb normal by wave map
	{
		vec2 uvA = (in_uv * u_waterFlowRepeat) - u_phaseA * in_waterVelocity * u_waterFlowOffset;
		vec2 uvB = (in_uv * u_waterFlowRepeat) - u_phaseB * in_waterVelocity * u_waterFlowOffset;
		//uvB += vec2(0.5);

		vec3 waveNrmA = waveNormal(vec3(uvA.x, in_worldPosition.y, uvA.y), 1.0 / 256.0, u_waveLevels, 0.22, u_waveChoppy);
		vec3 waveNrmB = waveNormal(vec3(uvB.x, in_worldPosition.y, uvB.y), 1.0 / 256.0, u_waveLevels, 0.22, u_waveChoppy);
		vec3 waveNrm = mix( waveNrmA, waveNrmB, u_phaseAlpha );
		waveNrm = normalize(waveNrm);

		float waveStrength = mix( 0.01, 5.0, length(in_waterVelocity) );

		vec3 tangent = cross( normal, vec3( 0.0, 0.0, 1.0 ) );
		vec3 bitangent = -cross( normal, tangent );

		normal += tangent * waveNrm.x * waveStrength;
		normal += bitangent * waveNrm.z * waveStrength;

		normal = normalize(normal);
	}

	vec3 outColor = vec3(0.0f);

	vec4 targetViewPosition = texelFetch(s_positionBuffer, ivec2(gl_FragCoord.xy));
	vec4 outputSample = texelFetch(s_output, ivec2(gl_FragCoord.xy));
	float viewDepth = abs( in_viewPosition.z - targetViewPosition.z );

	float filterAlpha = clamp(viewDepth / u_depthToFilter, 0.0, 1.0);
	float diffuseAlpha = clamp(viewDepth / u_depthToDiffuse, 0.0, 1.0);

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

		vec2 dimensions = vec2(textureSize(s_output, 0));
		outColor = mix(outputSample.rgb, texture2DRect(s_output, samplePos.xy * dimensions).rgb, filterAlpha * in_reflectAlpha);
	}
	
	////////////////////////////////////////////////////////////////
	// Filtering
	////////////////////////////////////////////////////////////////
	{
		outColor *= mix(vec3(1.0f), u_waterColor, filterAlpha);
	}
	
	////////////////////////////////////////////////////////////////
	// Diffuse response - due to dissolved dirt
	////////////////////////////////////////////////////////////////
	{
		vec3 dissolvedDirtColor = pow( vec3(1.0,0.0,0.0), vec3(2.2) );
		dissolvedDirtColor *= 0.25;
		outColor = mix( outColor, dissolvedDirtColor * diffuseLighting, in_dissolvedDirtAlpha * diffuseAlpha * in_reflectAlpha );
	}

	////////////////////////////////////////////////////////////////
	// Foam
	////////////////////////////////////////////////////////////////
	{
		vec2 uvA = (in_uv * u_foamRepeat) - u_phaseA * in_waterVelocity * u_waterFlowOffset * 1.0;
		vec2 uvB = (in_uv * u_foamRepeat) - u_phaseB * in_waterVelocity * u_waterFlowOffset * 1.0;
		uvB += vec2(0.5);

		float foamSampleA = texture(s_foamMap, uvA).x;
		float foamSampleB = texture(s_foamMap, uvB).x;
		float foamSample = mix(foamSampleA, foamSampleB, u_phaseAlpha);
		
		float foamAlpha = foamSample * in_foamStrength + max( 0.0, foamSample - (1.0-in_foamStrength) );
		foamAlpha *= u_foamAlphaStrength;
		foamAlpha *= in_reflectAlpha;

		outColor = mix( outColor, vec3(diffuseLighting) * 0.5, foamAlpha );
	}

	////////////////////////////////////////////////////////////////
	// Specular reflections
	////////////////////////////////////////////////////////////////
	{
		float waterSpecular = lightingGGX(normal, eye, lightDir, u_specularPower, 0.0) * u_lightIntensity * 0.025;
		outColor += waterSpecular * in_reflectAlpha * in_specularOcclusion;
	}

	////////////////////////////////////////////////////////////////
	// Sky reflections
	////////////////////////////////////////////////////////////////
	{
		vec3 reflectVec = -reflect(eye, normal);
		float fresnel = clamp(1.0 - dot(normal,eye), 0.0, 1.0);
		fresnel = pow(fresnel,3.0) * 0.65;
		fresnel = mix(0.1, 1.0, fresnel);
		outColor += getSkyColor(reflectVec) * u_ambientLightIntensity * 0.5 * in_reflectAlpha * fresnel * in_specularOcclusion;
	}

	

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