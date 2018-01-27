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
	float in_dissolvedDirtAlpha;
	vec3 in_eyeVec;
	float in_specularOcclusion;
	vec2 in_waterVelocity;
	vec2 in_uv;
	float in_foamStrength;
	float in_fluxAmount;
	float in_localDepthRatio;
	float in_reflectAlpha;
};

// Uniforms ////////////////////////////////////////////////////

// Samplers
uniform sampler2DRect s_output;
uniform sampler2DRect s_positionBuffer;
uniform sampler2DRect s_normalBuffer;
uniform sampler2D s_heightData;
uniform sampler2D s_miscData;
uniform sampler2D s_foamMap;
uniform sampler2D s_moltenGradient;
uniform samplerCube s_envMap;
uniform samplerCube s_irrMap;

// Matrices
uniform mat4 u_mvpMatrix;
uniform mat4 u_viewMatrix;

// Material
uniform vec3 u_filterColor;
uniform vec3 u_scatterColor;
uniform float u_indexOfRefraction;
uniform float u_reflectivity;
uniform float u_depthToFilter;
uniform float u_depthToDiffuse;
uniform float u_depthPower;
uniform float u_localDepthValue;

// Flow
uniform float u_phaseA;
uniform float u_phaseB;
uniform float u_phaseAlpha;
uniform float u_waterFlowOffset;
uniform float u_waterFlowRepeat;

// Waves
const mat2 octave_m = mat2(1.6,1.2,-1.2,1.6);
uniform float u_waveTime;
uniform int u_waveLevels;
uniform float u_waveAmplitude;
uniform float u_waveFreqBase;
uniform float u_waveFreqScalar;
uniform float u_waveRoughness;
uniform float u_waveChoppy;
uniform float u_waveChoppyEase;

// Foam
uniform float u_foamRepeat;
uniform float u_foamStrength;

// Dirt
uniform float u_dirtColor;

// Lighting
uniform vec3 u_lightDir;
uniform float u_lightDistance;
uniform float u_lightIntensity;
uniform float u_ambientLightIntensity;

// Glow
uniform float u_glowScalar;
uniform float u_glowMipLevel = 4;
uniform float u_glowDistance = 0.1;

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_forwad;

////////////////////////////////////////////////////////////////
// STD Lib Functions
////////////////////////////////////////////////////////////////
vec3 pointLightContribution(vec3 N,	vec3 L,	vec3 V,	vec3 diffColor,	vec3 specColor,	float roughness, vec3 lightColor, float lightIntensity );
vec3 IBLContribution(vec3 N, vec3 V, vec3 diffColor, vec3 specColor, float roughness, samplerCube envMap, samplerCube irrMap, float lightIntensity, float ambientOcclusion);

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
    float freq = u_waveFreqBase;
    float amp = ampScalar;
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
		freq *= u_waveFreqScalar;
		amp *= u_waveRoughness;
        choppy = mix(choppy,1.0,u_waveChoppyEase);
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
	
	// Peturb normal by wave map
	if ( false )
	{
		vec2 uvA = in_uv - u_phaseA * in_waterVelocity * u_waterFlowOffset;
		vec2 uvB = in_uv - u_phaseB * in_waterVelocity * u_waterFlowOffset;
		
		float waveStrength = min( 1.0, mix( 0.01, 1.0, in_fluxAmount ) );
		float waveAmplitude = u_waveAmplitude * waveStrength;

		vec3 waveNrmA = waveNormal(vec3(uvA.x, 0.0, uvA.y), 1.0 / 256.0, u_waveLevels, waveAmplitude, u_waveChoppy);
		vec3 waveNrmB = waveNormal(vec3(uvB.x, 0.0, uvB.y), 1.0 / 256.0, u_waveLevels, waveAmplitude, u_waveChoppy);
		vec3 waveNrm = mix( waveNrmA, waveNrmB, u_phaseAlpha );
		waveNrm = normalize(waveNrm);

		vec3 tangent = cross( normal, vec3( 0.0, 0.0, 1.0 ) );
		vec3 bitangent = -cross( normal, tangent );

		normal += tangent * waveNrm.x * 0.5;
		normal += bitangent * waveNrm.z * 0.5;
		normal = normalize(normal);
	}

	vec3 outColor = vec3(0.0f);

	// Depth alphas
	vec4 targetViewPosition = texelFetch(s_positionBuffer, ivec2(gl_FragCoord.xy));
	vec4 outputSample = texelFetch(s_output, ivec2(gl_FragCoord.xy));
	float viewDepth = abs( in_viewPosition.z - targetViewPosition.z );
	float localDepthValue = mix( u_localDepthValue, 0.0, clamp( in_localDepthRatio-1.0, 0.0, 1.0 ) );
	float localViewDepth = clamp( mix( viewDepth, localDepthValue, min( in_localDepthRatio, 1.0 )), 0.0, 1.0 ); 

	float filterAlpha = clamp(localViewDepth / u_depthToFilter, 0.0, 1.0);
	filterAlpha = pow(filterAlpha, u_depthPower) * in_reflectAlpha;
	float scatterAlpha = clamp(localViewDepth / u_depthToDiffuse, 0.0, 1.0);
	scatterAlpha = pow(scatterAlpha, u_depthPower) * in_reflectAlpha;

	// Mix to rock normal as water depth gets thin
	vec3 terrainNormal = texelFetch(s_normalBuffer, ivec2(gl_FragCoord.xy)).xyz;
	normal = normalize( mix( terrainNormal, normal, in_reflectAlpha ) );

	////////////////////////////////////////////////////////////////
	// Refraction
	////////////////////////////////////////////////////////////////
	{
		// We do this first as we need to know what pixel in the frame-buffer we're blending with
		vec3 refractVec = refract(-eye, normal, u_indexOfRefraction);
		
		vec4 samplePos = vec4( in_worldPosition );
		samplePos.xyz += refractVec * localViewDepth;
		samplePos *= u_mvpMatrix;
		samplePos.xyz /= samplePos.w;
		samplePos.xy += 1.0;
		samplePos.xy *= 0.5;

		vec2 dimensions = vec2(textureSize(s_output, 0));
		outColor = texture2DRect(s_output, samplePos.xy * dimensions).rgb;
	}
	
	////////////////////////////////////////////////////////////////
	// Filtering
	////////////////////////////////////////////////////////////////
	{
		outColor = mix(outColor, outColor*u_filterColor, filterAlpha);
		outColor *= 1.0-filterAlpha;
	}

	////////////////////////////////////////////////////////////////
	// Scattering
	////////////////////////////////////////////////////////////////
	{
		vec3 reflectVec = -reflect(eye, normal);
		vec3 wrappedLighting = texture( s_irrMap, reflectVec ).rgb;
		outColor += (wrappedLighting * u_scatterColor * scatterAlpha);
	}
	
	////////////////////////////////////////////////////////////////
	// Diffuse response - due to dissolved dirt
	////////////////////////////////////////////////////////////////
	/*
	{
		vec3 dissolvedDirtColor = pow( vec3(1.0,0.0,0.0), vec3(2.2) );
		dissolvedDirtColor *= 0.25;
		outColor = mix( outColor, dissolvedDirtColor * diffuseLighting, in_dissolvedDirtAlpha * diffuseAlpha );
	}
	*/

	////////////////////////////////////////////////////////////////
	// Foam
	////////////////////////////////////////////////////////////////
	/*
	{
		vec2 uvA = (in_uv * u_foamRepeat) - u_phaseA * in_waterVelocity * u_waterFlowOffset * 1.0;
		vec2 uvB = (in_uv * u_foamRepeat) - u_phaseB * in_waterVelocity * u_waterFlowOffset * 1.0;
		uvB += vec2(0.5);

		float foamSampleA = texture(s_foamMap, uvA).x;
		float foamSampleB = texture(s_foamMap, uvB).x;
		float foamSample = mix(foamSampleA, foamSampleB, u_phaseAlpha);
		
		float foamAlpha = foamSample * in_foamStrength + max( 0.0, foamSample - (1.0-in_foamStrength) );
		foamAlpha *= u_foamStrength;
		foamAlpha *= in_reflectAlpha;

		outColor = mix( outColor, vec3(diffuseLighting) * 0.5, foamAlpha );
	}
	*/

	////////////////////////////////////////////////////////////////
	// Reflections
	////////////////////////////////////////////////////////////////
	{
		float roughness = 0.4;
		vec3 reflections = IBLContribution(normal, eye, vec3(0.0), vec3(u_reflectivity), roughness, s_envMap, s_irrMap, u_ambientLightIntensity, in_specularOcclusion * in_reflectAlpha);
		outColor += reflections;
	}

	////////////////////////////////////////////////////////////////
	// Local glow
	////////////////////////////////////////////////////////////////
	/*
	vec3 heatLight = vec3(0.0);
	{
		vec3 sampleOffset = normalize( vec3( normal.x, 0.0, normal.z ) );
		
		vec3 samplePos = in_worldPosition.xyz + sampleOffset * u_glowDistance;
		vec2 sampleUV = samplePos.xz + vec2(0.5);
		vec4 sampleHeightData = texture(s_heightData, sampleUV);
		samplePos.y = sampleHeightData.x + sampleHeightData.y + sampleHeightData.z;
		samplePos.y *= 2.0;

		float sampleHeat = textureLod( s_miscData, sampleUV, u_glowMipLevel ).x;
		vec3 sampleDir = samplePos - in_worldPosition.xyz;
		float sampleDis = length(sampleDir);
		sampleDir = normalize(sampleDir);

		vec3 heatColor = pow( texture(s_moltenGradient, vec2(sampleHeat * 0.5, 0.5)).rgb, vec3(2.2) );
		vec3 sampleHeatLight = pointLightContribution( normal, sampleDir, eye, vec3(0.0), vec3(u_reflectivity), 0.25, heatColor, u_glowScalar);
		sampleHeatLight /= (1.0 + sampleDis*sampleDis);

		heatLight += sampleHeatLight;
	}
	outColor += heatLight;
	*/

	////////////////////////////////////////////////////////////////
	// Debug
	////////////////////////////////////////////////////////////////
	/*
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