#version 430 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Samplers
uniform sampler2D s_rockData;
uniform sampler2D s_rockFluxData;
uniform sampler2D s_rockNormalData;

uniform sampler2D s_waterData;
uniform sampler2D s_waterFluxData;
uniform sampler2D s_waterNormalData;

uniform sampler2D s_mappingData;
uniform sampler2D s_diffuseMap;

// From VS
in Varying
{
	vec2 in_uv;
};

// Uniforms

// Mouse
uniform vec2 u_mousePos;
uniform float u_mouseRadius;
uniform float u_mouseMoltenVolumeStrength;
uniform float u_mouseWaterVolumeStrength;
uniform float u_mouseMoltenHeatStrength;

// Environment
uniform float u_ambientTemp;

// Molten
uniform float u_heatAdvectSpeed;
uniform float u_viscosityMin;
uniform float u_viscosityMax;
uniform float u_heatViscosityPower;
uniform float u_heatViscosityBias;

uniform float u_tempChangeSpeed;
uniform float u_condenseSpeed;
uniform float u_meltSpeed;

// Water
uniform float u_waterViscosity;
uniform float u_erosionStrength;
uniform float u_erosionFluxMin;
uniform float u_erosionFluxMax;
uniform float u_airErosionStrength;
uniform float u_erosionMaxDepth;
uniform float u_airErosionMinDiff;
uniform float u_dirtTransportSpeed;

// Waves
uniform float u_wavePhase;
uniform float u_waveFrequency;
uniform float u_waveFrequencyLacunarity;
uniform float u_waveAmplitude;
uniform float u_waveAmplitudeLacunarity;
uniform float u_waveChoppy;
uniform float u_waveChoppyEase;
uniform int u_waveOctavesNum;
uniform float u_waveDepthMax;


// Buffers
layout( std430, binding = 0 ) buffer MousePositionBuffer
{
	int mouseBufferZ;
	int mouseBufferU;
	int mouseBufferV;
};


////////////////////////////////////////////////////////////////
// Outputs
////////////////////////////////////////////////////////////////

layout( location = 0 ) out vec4 out_rockData;
layout( location = 1 ) out vec4 out_waterData;

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
//
float hash( vec2 p ) 
{
	float h = dot(p,vec2(127.1,311.7));	
    return fract(sin(h)*43758.5453123);
}

////////////////////////////////////////////////////////////////
//
float noise( in vec2 p ) 
{
    vec2 i = floor( p );
    vec2 f = fract( p );	
	vec2 u = f*f*(3.0-2.0*f);
	float ret = mix( mix( hash( i + vec2(0.0,0.0) ), hash( i + vec2(1.0,0.0) ), u.x),
                     mix( hash( i + vec2(0.0,1.0) ), hash( i + vec2(1.0,1.0) ), u.x), u.y);
	return -1.0+2.0*ret;
}

////////////////////////////////////////////////////////////////
//
float waveOctave(vec2 uv, float choppy)
{
    uv += noise(uv);        
    vec2 wv = 1.0-abs(sin(uv));
    vec2 swv = abs(cos(uv));    
    wv = mix(wv,swv,wv);
    return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
}

////////////////////////////////////////////////////////////////
//
mat2 octave_m = mat2(1.6,1.2,-1.2,1.6);
float waveNoise(vec2 p) 
{
    float freq = u_waveFrequency;
    float amp = 1.0;
    float choppy = u_waveChoppy;
    
    float d;
	float h = 0.0; 

    for(int i = 0; i < u_waveOctavesNum; i++) 
	{        
    	d = waveOctave((p+u_wavePhase)*freq,choppy);
    	d += waveOctave((p-u_wavePhase)*freq,choppy);
        h += d * amp;
    	p *= octave_m; 
		freq *= u_waveFrequencyLacunarity;
		amp *= u_waveAmplitudeLacunarity;
        choppy = mix(choppy,1.0,u_waveChoppyEase);
    }

    return (h-0.5)*2.0;
}

////////////////////////////////////////////////////////////////
//
vec2 GetMousePos()
{
	vec2 mousePos = vec2( mouseBufferU, mouseBufferV ) / 255;
	return mousePos;
}

////////////////////////////////////////////////////////////////
//
float CalcViscosity( float _heat, float _viscosityScalar )
{
	float viscosity = mix( u_viscosityMin, u_viscosityMax, 1.0-_viscosityScalar );
	return pow(smoothstep( 0.0f, 1.0f, clamp(_heat-u_heatViscosityBias, 0.0f, 1.0)), u_heatViscosityPower) * viscosity;
}

////////////////////////////////////////////////////////////////
//
vec2 VelocityFromFlux( vec4 fluxC, vec4 fluxL, vec4 fluxR, vec4 fluxU, vec4 fluxD, float viscosity )
{
	return vec2((fluxL.y + fluxC.y) - (fluxR.x + fluxC.x), 
				(fluxU.w + fluxC.w) - (fluxD.z + fluxC.z) ) * viscosity;
}

////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////

void main(void)
{ 
	const vec4 EPSILON = vec4(0.001f);

	ivec2 dimensions = textureSize( s_rockData, 0 );
	vec2 texelSize = 1.0f / dimensions;
	ivec2 texelCoordC = ivec2(gl_FragCoord.xy);
	ivec2 texelCoordL = texelCoordC - ivec2(1,0);
	ivec2 texelCoordR = texelCoordC + ivec2(1,0);
	ivec2 texelCoordU = texelCoordC - ivec2(0,1);
	ivec2 texelCoordD = texelCoordC + ivec2(0,1);

	// Shared data samples
	vec4 rockDataC = texelFetch(s_rockData, texelCoordC, 0);
	vec4 waterDataC = texelFetch(s_waterData, texelCoordC, 0);
	vec4 mappingDataC = texelFetch(s_mappingData, texelCoordC, 0);
	vec4 diffuseSampleC = texture(s_diffuseMap, in_uv);
	
	// Shared local vars
	float solidHeight = rockDataC.x;
	float dirtHeight = rockDataC.w;
	float moltenHeight = rockDataC.y;
	float moltenHeat = rockDataC.z;
	float iceHeight = waterDataC.x;
	float waterHeight = waterDataC.y;

	float moltenViscosityScalar = mappingDataC.y;
	float moltenViscosity = CalcViscosity(moltenHeat, moltenViscosityScalar);

	vec2 mousePos = GetMousePos();
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-mousePos) / u_mouseRadius);

	////////////////////////////////////////////////////////////////
	// Update molten
	////////////////////////////////////////////////////////////////
	float newMoltenHeight = moltenHeight;
	float newMoltenHeat = moltenHeat;

	{
		vec4 fluxC = texelFetch(s_rockFluxData, texelCoordC, 0);
		vec4 fluxL = texelFetch(s_rockFluxData, texelCoordL, 0);
		vec4 fluxR = texelFetch(s_rockFluxData, texelCoordR, 0);
		vec4 fluxU = texelFetch(s_rockFluxData, texelCoordU, 0);
		vec4 fluxD = texelFetch(s_rockFluxData, texelCoordD, 0);
		vec4 fluxN = vec4(fluxL.y, fluxR.x, fluxU.w, fluxD.z);

		// Update molten height based on flux
		float fluxChange = ((fluxN.x+fluxN.y+fluxN.z+fluxN.w)-(fluxC.x+fluxC.y+fluxC.z+fluxC.w));
		newMoltenHeight = moltenHeight + fluxChange * moltenViscosity;
		
		// What proportion of our volume did we lose to neighbours?
		// If we lose half our volume, we also lose half our heat.
		float volumeLossProp = ((fluxC.x+fluxC.y+fluxC.z+fluxC.w)* moltenViscosity) / (moltenHeight + 0.001);
		volumeLossProp = min( 1.0f, volumeLossProp );
		newMoltenHeat -= (volumeLossProp * moltenHeat) * u_heatAdvectSpeed;

		// For each neighbour, determine what proportion of their volume we have gained.
		// We also want to grab the same proportion of their heat.
		// Essentially the inverse of above.
		vec4 rockDataL = texelFetch(s_rockData, texelCoordL, 0);
		vec4 rockDataR = texelFetch(s_rockData, texelCoordR, 0);
		vec4 rockDataU = texelFetch(s_rockData, texelCoordU, 0);
		vec4 rockDataD = texelFetch(s_rockData, texelCoordD, 0);
		vec4 mappingDataL = texelFetch(s_mappingData, texelCoordL, 0);
		vec4 mappingDataR = texelFetch(s_mappingData, texelCoordR, 0);
		vec4 mappingDataU = texelFetch(s_mappingData, texelCoordU, 0);
		vec4 mappingDataD = texelFetch(s_mappingData, texelCoordD, 0);

		vec4 neighbourHeat = vec4(rockDataL.z, rockDataR.z, rockDataU.z, rockDataD.z);
		vec4 neighbourHeight = vec4(rockDataL.y, rockDataR.y, rockDataU.y, rockDataD.y);
		vec4 neighbourViscosity = vec4( 
			CalcViscosity(neighbourHeat.x, mappingDataL.y), 
			CalcViscosity(neighbourHeat.y, mappingDataR.y), 
			CalcViscosity(neighbourHeat.z, mappingDataU.y), 
			CalcViscosity(neighbourHeat.w, mappingDataD.y) 
		);
		vec4 volumeGainProp = (fluxN * neighbourViscosity) / (neighbourHeight + EPSILON);
		volumeGainProp = min( vec4(1.0), volumeGainProp );
		vec4 heatGain = volumeGainProp * neighbourHeat;
		newMoltenHeat += (heatGain.x + heatGain.y + heatGain.z + heatGain.w) * u_heatAdvectSpeed;

		// Cooling
		// Occluded areas cool slower
		vec4 rockNormalDataC = texelFetch(s_rockNormalData, texelCoordC, 0);
		float occlusion = rockNormalDataC.w;
		newMoltenHeat += (u_ambientTemp - newMoltenHeat) * u_tempChangeSpeed * (1.0-occlusion);

		// Add some lava near the mouse
		float mouseTextureScalar = diffuseSampleC.y;
		float mouseTextureScalar2 = 1.0-diffuseSampleC.x;
		newMoltenHeat	+= ( pow(mouseRatio, 1.0) * u_mouseMoltenHeatStrength   * mix(0.2, 1.0, mouseTextureScalar) ) / (1.0001+moltenHeat*5.0);
		newMoltenHeight += ( pow(mouseRatio, 1.5) * u_mouseMoltenVolumeStrength * mix(0.2, 1.0, mouseTextureScalar2) );// / (1.0001+newMoltenHeight);
	}

	////////////////////////////////////////////////////////////////
	// Update water
	////////////////////////////////////////////////////////////////
	float newWaterHeight = waterHeight;
	float waveNoiseHeight;
	float newDirtHeight = dirtHeight;
	{
		vec4 fluxC = texelFetch(s_waterFluxData, texelCoordC, 0);
		vec4 fluxL = texelFetch(s_waterFluxData, texelCoordL, 0);
		vec4 fluxR = texelFetch(s_waterFluxData, texelCoordR, 0);
		vec4 fluxU = texelFetch(s_waterFluxData, texelCoordU, 0);
		vec4 fluxD = texelFetch(s_waterFluxData, texelCoordD, 0);
		vec4 fluxN = vec4(fluxL.y, fluxR.x, fluxU.w, fluxD.z);

		// Update water height based on flux
		float fluxChange = ((fluxN.x+fluxN.y+fluxN.z+fluxN.w)-(fluxC.x+fluxC.y+fluxC.z+fluxC.w));
		newWaterHeight = waterHeight + fluxChange * u_waterViscosity;

		// Add some water near the mouse
		float mouseTextureScalar = diffuseSampleC.x;
		newWaterHeight += pow(mouseRatio, 1.1) * u_mouseWaterVolumeStrength * mix(0.9, 1.0, mouseTextureScalar);

		////////////////////////////////////////////////////////////////
		// Wave noise
		////////////////////////////////////////////////////////////////

		waveNoiseHeight = waveNoise(in_uv);
		waveNoiseHeight *= u_waveAmplitude;

		float mippedWaterHeight = texture2D(s_waterData, in_uv, 4).y;
		float heightRatio = smoothstep( 0.0, u_waveDepthMax, mippedWaterHeight);
		waveNoiseHeight *= heightRatio;

		// Scale by choppyness
		waveNoiseHeight *= mappingDataC.z;

		////////////////////////////////////////////////////////////////
		// Dirt transport
		////////////////////////////////////////////////////////////////
		vec2 waterVelocity = VelocityFromFlux( fluxC, fluxL, fluxR, fluxU, fluxD, u_waterViscosity );

		float waterVolumeScalar = min(newWaterHeight / 0.01, 1.0);
		waterVelocity *= waterVolumeScalar;

		//float waterSpeedL = smoothstep( u_erosionFluxMin, u_erosionFluxMax, max( -waterVelocity.x, 0.0 ) );
		//float waterSpeedR = smoothstep( u_erosionFluxMin, u_erosionFluxMax, max(  waterVelocity.x, 0.0 ) );
		//float waterSpeedU = smoothstep( u_erosionFluxMin, u_erosionFluxMax, max( -waterVelocity.y, 0.0 ) );
		//float waterSpeedD = smoothstep( u_erosionFluxMin, u_erosionFluxMax, max(  waterVelocity.y, 0.0 ) );

		float waterSpeedL = clamp( -waterVelocity.x / u_erosionFluxMax, 0.0, 1.0 );
		float waterSpeedR = clamp(  waterVelocity.x / u_erosionFluxMax, 0.0, 1.0 );
		float waterSpeedU = clamp( -waterVelocity.y / u_erosionFluxMax, 0.0, 1.0 );
		float waterSpeedD = clamp(  waterVelocity.y / u_erosionFluxMax, 0.0, 1.0 );

		float availableDirtC = newDirtHeight*0.25;	// A quarter, so each direction can get its fair share

		float toLeft  = min( waterSpeedL * u_dirtTransportSpeed, availableDirtC );
		float toRight = min( waterSpeedR * u_dirtTransportSpeed, availableDirtC );
		float toAbove = min( waterSpeedU * u_dirtTransportSpeed, availableDirtC );
		float toBelow = min( waterSpeedD * u_dirtTransportSpeed, availableDirtC );

		float availableDirtL = texelFetch(s_rockData, texelCoordL, 0).w * 0.25;
		float availableDirtR = texelFetch(s_rockData, texelCoordR, 0).w * 0.25;
		float availableDirtU = texelFetch(s_rockData, texelCoordU, 0).w * 0.25;
		float availableDirtD = texelFetch(s_rockData, texelCoordD, 0).w * 0.25;

		float fromLeft  = min( waterSpeedR * u_dirtTransportSpeed, availableDirtL );
		float fromRight = min( waterSpeedL * u_dirtTransportSpeed, availableDirtR );
		float fromAbove = min( waterSpeedD * u_dirtTransportSpeed, availableDirtU );
		float fromBelow = min( waterSpeedU * u_dirtTransportSpeed, availableDirtD );
													  
		newDirtHeight -= (toLeft+toRight+toAbove+toBelow);
		newDirtHeight += (fromLeft+fromRight+fromAbove+fromBelow);

		////////////////////////////////////////////////////////////////
		// Water erosion
		////////////////////////////////////////////////////////////////
		float waterSpeed = length(waterVelocity);
		//float erosionDepthScalar = 1.0 - smoothstep( 0.0, u_erosionMaxDepth, newDirtHeight );
		//float erosionSpeedScalar = smoothstep( u_erosionFluxMin, u_erosionFluxMax, waterSpeed );
		float erosionDepthScalar = 1.0 - min(newDirtHeight / u_erosionMaxDepth, 1.0);
		float erosionSpeedScalar = min(waterSpeed / u_erosionFluxMax, 1.0);
		float rockToDirt = erosionSpeedScalar * u_erosionStrength * solidHeight * erosionDepthScalar;

		solidHeight -= rockToDirt;
		newDirtHeight += rockToDirt;


		////////////////////////////////////////////////////////////////
		// Air erosion
		////////////////////////////////////////////////////////////////
		vec4 rockDataL = texelFetch(s_rockData, texelCoordL, 0);
		vec4 rockDataR = texelFetch(s_rockData, texelCoordR, 0);
		vec4 rockDataU = texelFetch(s_rockData, texelCoordU, 0);
		vec4 rockDataD = texelFetch(s_rockData, texelCoordD, 0);
		float heightC = solidHeight;
		float heightL = rockDataL.x;
		float heightR = rockDataR.x;
		float heightU = rockDataU.x;
		float heightD = rockDataD.x;

		float smallestDiff = min( min(heightC-heightL, heightC-heightR), min(heightC-heightU, heightC-heightD ) );
		float scalar = step( u_airErosionMinDiff, abs(smallestDiff) );
		solidHeight -= smallestDiff * u_airErosionStrength * scalar;
	}

	////////////////////////////////////////////////////////////////
	// Melt/condense rock
	////////////////////////////////////////////////////////////////
	{
		float maxTemp = u_heatViscosityBias * 5.0;

		float solidToMolten = smoothstep(u_heatViscosityBias, maxTemp, newMoltenHeat) * u_meltSpeed;
		solidToMolten = min(solidToMolten, solidHeight);

		float moltenToSolid = (1.0-smoothstep(0, u_heatViscosityBias, newMoltenHeat)) * u_condenseSpeed;
		moltenToSolid = min(moltenToSolid, newMoltenHeight);

		solidHeight -= solidToMolten;
		solidHeight += moltenToSolid;

		newMoltenHeight  -= moltenToSolid;
		newMoltenHeight  += solidToMolten;
	}
	
	////////////////////////////////////////////////////////////////
	// Output
	////////////////////////////////////////////////////////////////
	out_rockData = vec4(solidHeight, newMoltenHeight, newMoltenHeat, newDirtHeight);
	out_waterData = vec4(0.0, newWaterHeight, 0.0, waveNoiseHeight);
}








