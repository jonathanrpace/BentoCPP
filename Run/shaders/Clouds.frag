#version 330 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

#define M_PI 3.1415926535897932384626433832795

// Uniforms

uniform vec3 lightSamplePositions[] = vec3[](
	vec3(0.0, 0.0, 0.1),
	vec3(0.0, 0.0, 0.2),
	vec3(0.0, 0.0, 0.35),
	vec3(0.0, 0.0, 0.5),
	vec3(0.1, 0.0, 0.7),
	vec3(0.0, 0.0, 12.0)
);


// Transform
uniform float u_baseScale;
uniform float u_baseScaleVertical;
uniform float u_detailScale;
uniform float u_detailScaleVertical;
uniform float u_position;
uniform float u_height;
uniform float u_falloffTop;
uniform float u_falloffBottom;

// Vortex
uniform float u_numVortexTurns;

// Quality
uniform float u_maxRayLength;
uniform int u_coarseStepsPerUnit;
uniform int u_detailStepsPerUnit;
uniform float u_lightConeMaxLength;
uniform mat3 u_coneMatrix;

// Lighting
uniform float u_lightIntensity;
uniform vec3 u_lightColor;
uniform vec3 u_skyColor;

// Properties
uniform float u_absorbtion;
uniform float u_powderSharpness;
uniform float u_scatteringParam;
uniform float u_densityScalar;
uniform float u_densityOffset;
uniform float u_detailMaxDensity;
uniform float u_detailDensityScalar;
uniform float u_detailDensityOffset;

// 
uniform vec3 u_cameraPos;


// Samplers
uniform sampler3D s_baseShapes;
uniform sampler3D s_detailMap;

// Varying
in Varying
{
	vec4 in_rayNear;
	vec4 in_rayFar;
};

////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_forward;

////////////////////////////////////////////////////////////////
// Lib Functions
////////////////////////////////////////////////////////////////
vec2 rotateVec2(vec2 vec, float radians);
vec3 screenSpaceToEyeSpace( vec4 viewport, mat4 invPersMatrix );

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

float Beers(float density, float absorbtion) 
{
	return pow( 2.0, -density*absorbtion );
}	

float Powder( float density )
{
	return 1.0 - pow( 2.0, -density*u_powderSharpness );
}

float HG(float theta, float g)	// g is scattering param. Where -1 = back scattering - 0 = isotropic - 1 = forward scattering
{
	return (0.25*M_PI) * ( (1-g*g) / pow(1+g*g-(2*g*cos(theta)), 3/2) );
}

float layerGradient( vec3 p )
{
	float bottom = u_position-(u_height*0.5);
	if ( p.y < bottom )
		return 0.0;
	
	float top = u_position+(u_height*0.5);
	
	if ( p.y > top )
		return 0.0;
		
	return smoothstep( bottom, bottom + u_falloffBottom, p.y ) * (1.0 - smoothstep( top - u_falloffTop, top, p.y ));
}

float getDensityBase( vec3 p )
{
	float layerScalar = layerGradient(p);
	
	if ( layerScalar == 0.0 )
		return 0.0;
	
	vec3 coord = p.xzy * u_baseScale;
	coord.z *= u_baseScaleVertical;
	vec4 s = texture( s_baseShapes, coord);
	//float v = (s.r + s.g * 0.5 + s.b * 0.25) / 1.75;
	float v = s.r - (1.0-s.g) * 0.5 - (1.0-s.b) * 0.25;
	
	v = (v-u_densityOffset) * u_densityScalar;

	v = max( 0, v );
	v *= layerScalar;
	
	return v;
}

float getDetailDensity( vec3 p )
{
	vec3 coord = p.xzy * u_detailScale;
	coord.z *= u_detailScaleVertical;
	vec4 s = texture( s_detailMap, coord );
	float v = (s.a-u_detailDensityOffset) * u_detailDensityScalar;
	v = max( 0, v );
	
	return v;
}

vec3 warpPosition( vec3 p )
{
	return p;
	
	vec2 origin = vec2(0.0);
	vec2 delta = vec2(p.x, p.z) - origin;
	float r = length(delta);
	
	float radians = u_numVortexTurns * M_PI * 2.0 / (1.0+r);
	
	vec2 rotatedDelta = rotateVec2(delta, radians);
	
	vec2 warpedPos = origin + rotatedDelta / (1.0+r);
	
	return vec3(warpedPos.x, p.y - r*0.25, warpedPos.y);
}

float getDensityLOD( vec3 p, bool doDetail )
{
	p = warpPosition(p);

	float density = getDensityBase(p);
	
	if ( density > 0.0 && doDetail && density < u_detailMaxDensity )
	{
		float detailRatio = 1.0-smoothstep( 0.0, u_detailMaxDensity, density );
		float detailDensity = getDetailDensity(p);
		density = mix( density, density * (1.0-detailDensity), detailRatio );
		density = max(0.0, density);
	}
	
	return density;
}

////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////
void main(void)
{
	vec3 rayDir = normalize(in_rayFar.xyz - in_rayNear.xyz);
	vec3 lightDir = vec3(0.0,0.0,1.0) * u_coneMatrix;
	
	
	float thetaRatio = dot(-rayDir, lightDir);
	float theta = acos(thetaRatio); // Angle between eye vector and light
	thetaRatio = max(0.0, thetaRatio);
	
	int detailToCoarseMultiplier = 3;
	
	int numCoarseSteps = int(u_maxRayLength * u_coarseStepsPerUnit);
	float coarseStepLength = u_maxRayLength / numCoarseSteps;
	
	int numDetailSteps = numCoarseSteps * detailToCoarseMultiplier;
	float detailStepLength = u_maxRayLength / numDetailSteps;
	
	int numLightSamples = lightSamplePositions.length();
	vec3 lightEnergy = u_lightColor * u_lightIntensity;
	
	// TODO Consider renaming to 'primary' and 'seconday' rays. Instead of 'density' and 'energy'
	
	vec3 rayPos = u_cameraPos;
	float rayLength = 0.0;
	float extinction = 1.0f;
	float accumulatedDensity = 0.0;
	vec3 accumulatedEnergy = vec3(0.0);
	float stepSize = coarseStepLength;
	bool detailMode = false;
	int numZeroDetailSteps = 0;
	while ( rayLength < u_maxRayLength )
	{
		// Get density - only do detail if making small steps, and if not too obscured already
		bool extinctionLowEnoughForDetail = extinction > 0.8;
		float density = getDensityLOD(rayPos, detailMode);
		
		// Cheaply continue in empty regions
		
		if ( density == 0.0 )
		{
			// If we've had more than empty detail steps than fit into a single coarse step,
			// switch back to coarse steps
			if ( detailMode )
			{
				numZeroDetailSteps++;
				if ( numZeroDetailSteps > detailToCoarseMultiplier )
				{
					detailMode = false;
					stepSize = coarseStepLength;
				}
			}
		}
		else
		{
			// We've been making coarse steps and have hit something.
			// Back up to previous position, and continue with detailed steps
			if ( !detailMode )
			{
				rayPos -= rayDir * stepSize;
				rayLength -= stepSize;
		
				detailMode = true;
				stepSize = detailStepLength;
				numZeroDetailSteps = 0;
			}
			// We have non zero density here.
			// Continue marching forward with detailed steps
			else
			{
				density *= stepSize;
				accumulatedDensity += density;
				extinction *= Beers( accumulatedDensity, u_absorbtion );
				
				// Stop when extinction is high
				if ( extinction < 0.02 )
					break;
				
				float accumulatedLightSampleDensity = 0.0;
				for ( int j = 0; j < numLightSamples; j++ )
				{
					vec3 lightSamplePos = lightSamplePositions[j];
					lightSamplePos = -lightSamplePos * u_coneMatrix;
					lightSamplePos *= u_lightConeMaxLength;
					lightSamplePos += rayPos;
					
					float lightSampleDensity = getDensityLOD(lightSamplePos, extinctionLowEnoughForDetail);// * u_lightConeMaxLength;
					accumulatedLightSampleDensity += lightSampleDensity;
				}
				accumulatedLightSampleDensity /= numLightSamples;
				float lightTransmitRatio = Beers( accumulatedLightSampleDensity, u_absorbtion );
				lightTransmitRatio += HG(theta, u_scatteringParam);
				float powderedLightTransmitRatio = lightTransmitRatio * Powder( accumulatedLightSampleDensity );
				float powderedRatio = mix( 0.4, 1.0, 1.0-pow( thetaRatio, 4.0 ) );
				lightTransmitRatio = mix( lightTransmitRatio, powderedLightTransmitRatio, powderedRatio );
				
				//accumulatedEnergy += lightTransmitRatio * (1.0-extinction) * stepSize;
				
				vec3 col = vec3( lightEnergy * lightTransmitRatio ) * extinction * stepSize;
				accumulatedEnergy += col*(1.0-extinction);
			}
		}
		
		rayPos += rayDir * stepSize;
		rayLength += stepSize;
	}
	
	//vec3 cloudColor = accumulatedEnergy * lightEnergy;
	
	accumulatedEnergy += u_skyColor * u_lightIntensity * 0.1;
	vec3 backgroundColor = mix( u_skyColor, lightEnergy, pow( thetaRatio, 10.0 ) );
	vec3 outColor = mix( accumulatedEnergy, backgroundColor, extinction );
	
	//vec3 outColor = accumulatedEnergy + vec3(extinction) * backgroundColor;
	
	out_forward = vec4( accumulatedEnergy, 1.0-extinction );
}
