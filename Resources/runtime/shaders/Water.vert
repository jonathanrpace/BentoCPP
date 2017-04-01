#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_rand;

// Textures
uniform sampler2D s_heightData;
uniform sampler2D s_velocityData;
uniform sampler2D s_miscData;
uniform sampler2D s_normalData;
uniform sampler2D s_smudgeData;
uniform sampler2D s_fluxData;
 
// Uniforms
uniform mat4 u_mvpMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat4 u_viewMatrix;

uniform float u_heightOffset;
uniform float u_depthToReflect;
uniform float u_dissolvedDirtDensityScalar;

// Flow
uniform float u_phaseA;
uniform float u_phaseB;
uniform float u_phaseAlpha;
uniform float u_waterFlowOffset;
uniform float u_waterFlowRepeat;

// Waves
const mat2 octave_m = mat2(1.6,1.2,-1.2,1.6);
uniform int u_waveLevels;
uniform float u_waveTime;
uniform float u_waveAmplitude;
uniform float u_waveFreqBase;
uniform float u_waveFreqScalar;
uniform float u_waveRoughness;
uniform float u_waveChoppy = 2.0;


////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

// GL
out gl_PerVertex 
{
	vec4 gl_Position;
};

// Varying
out Varying
{
	vec3 out_normal;
	vec4 out_viewPosition;
	vec4 out_worldPosition;
	float out_reflectAlpha;
	float out_dissolvedDirtAlpha;
	vec3 out_eyeVec;
	float out_specularOcclusion;
	vec2 out_waterVelocity;
	vec2 out_uv;
	float out_foamStrength;
	float out_fluxAmount;
};

////////////////////////////////////////////////////////////////
// STD Lib Functions
////////////////////////////////////////////////////////////////
vec3 reconstructNormal( vec2 n )
{
	float len = length(n);
	return normalize( vec3(n.x, (1.0-len), n.y) );
}

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
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////
void main(void)
{
	// Pluck some values out of the texture data
	vec4 heightDataC = texture(s_heightData, in_uv);
	vec4 velocityDataC = texture(s_velocityData, in_uv);
	vec4 miscDataC = texture(s_miscData, in_uv);
	vec4 normalDataC = texture(s_normalData, in_uv);
	vec4 smudgeDataC = texture(s_smudgeData, in_uv);
	vec4 fluxDataC = texture(s_fluxData, in_uv);

	vec4 absFlux = abs(fluxDataC);
	float fluxAmount = (absFlux.x + absFlux.y + absFlux.z + absFlux.w) * 100.0;
	out_fluxAmount = fluxAmount;

	float solidHeight = heightDataC.x;
	float moltenHeight = heightDataC.y;
	float dirtHeight = heightDataC.z;
	float waterHeight = heightDataC.w;
	float dissolvedDirt = miscDataC.z;

	out_foamStrength = min( smudgeDataC.w, 1.0 );

	vec2 waterVelocity = velocityDataC.zw;
	out_waterVelocity = waterVelocity;
	out_uv = in_uv;

	vec3 normal = reconstructNormal(normalDataC.xy);
	out_normal = normal;

	vec4 position = vec4(in_position, 1.0f);
	position.y += solidHeight;
	position.y += moltenHeight;
	position.y += dirtHeight;
	position.y += waterHeight;
	
	{
		vec2 uvA = (in_uv * u_waterFlowRepeat) - u_phaseA * waterVelocity * u_waterFlowOffset;
		vec2 uvB = (in_uv * u_waterFlowRepeat) - u_phaseB * waterVelocity * u_waterFlowOffset;
		
		float waveStrength = min( 1.0, mix( 0.01, 1.0, fluxAmount ) );
		float waveAmplitude = u_waveAmplitude * waveStrength;

		float waveHeightA = waveNoise(vec3(uvA.x, 0, uvA.y), u_waveLevels, waveAmplitude, u_waveChoppy);
		float waveHeightB = waveNoise(vec3(uvB.x, 0, uvB.y), u_waveLevels, waveAmplitude, u_waveChoppy);
		float waveHeight = mix( waveHeightA, waveHeightB, u_phaseAlpha );

		waterHeight += waveHeight;
		position.y += waveHeight;
	}

	out_worldPosition = position;

	float reflectAlpha = min( max(waterHeight, 0.0) / u_depthToReflect, 1.0 );
	out_reflectAlpha = reflectAlpha;

	vec4 viewPosition = u_modelViewMatrix * position;
	viewPosition.w = 1.0;
	out_viewPosition = viewPosition;

	vec4 screenPos = u_mvpMatrix * position;
	gl_Position = screenPos;

	////////////////////////////////////////////////////////////////
	// Dissolved Dirt Alpha
	////////////////////////////////////////////////////////////////
	{
		float dissolvedDirtAlpha = min( dissolvedDirt * u_dissolvedDirtDensityScalar, 1.0 );
		out_dissolvedDirtAlpha = dissolvedDirtAlpha;
	}

	////////////////////////////////////////////////////////////////
	// Specular occlusion
	////////////////////////////////////////////////////////////////
	{
		out_eyeVec = -normalize( viewPosition.xyz * mat3(u_viewMatrix) );
		vec3 reflectVec = -reflect(out_eyeVec, normal);

		// Specular occlusion
		float shadowing = 0.0;
		{
			const int maxSteps = 64;
			const float minStepLength = (1.0 / 256.0);

			float stepLength = (1.0 / maxSteps);
			float stepLengthScalar = 1.0;

			vec3 rayPos = position.xyz;
			vec3 rayDir = reflectVec;

			for ( int i = 0; i < maxSteps; i++ )
			{
				rayPos += rayDir * stepLength;

				vec2 sampleUV = rayPos.xz + 0.5;

				vec4 heightSample = texture(s_heightData, sampleUV);
				float terrainHeightAtSample = heightSample.x + heightSample.y + heightSample.z;

				if ( terrainHeightAtSample > rayPos.y )
				{
					if ( i == (maxSteps-1) || stepLength <= minStepLength )
					{
						shadowing = 1.0;
						break;
					}

					stepLengthScalar = 0.5;
					rayPos -= rayDir * stepLength;
				}

				stepLength *= stepLengthScalar;
			}
		}
		
		out_specularOcclusion = 1.0;//1.0-shadowing;
	}
} 