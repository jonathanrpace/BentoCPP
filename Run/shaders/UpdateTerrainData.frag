#version 430 core

const vec4 EPSILON = vec4(0.000001f);
const float PI = 3.14159265359;
const float HALF_PI = PI * 0.5f;
////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// From VS
in Varying
{
	vec2 in_uv;
};

// Uniforms

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_moltenFluxData;
uniform sampler2D s_waterFluxData;
uniform sampler2D s_miscData;
uniform sampler2D s_smudgeData;
uniform sampler2D s_uvOffsetData;
uniform sampler2D s_derivedData;
uniform sampler2D s_pressureData;
uniform sampler2D s_noiseMap;

// Mouse
uniform float u_cellSize;
uniform float u_mouseRadius;
uniform float u_mouseMoltenVolumeStrength;
uniform float u_mouseWaterVolumeStrength;
uniform float u_mouseMoltenHeatStrength;
uniform float u_mouseDirtVolumeStrength;

// Environment
uniform float u_ambientTemp;

// Global
uniform float u_phase;
uniform bool u_phaseALatch;
uniform bool u_phaseBLatch;

// Molten
uniform float u_moltenMinHeat;
uniform float u_tempChangeSpeed;
uniform float u_meltSpeed;
uniform float u_condenseSpeed;
uniform float u_smudgeChangeRate;
uniform float u_moltenDiffuseStrength = 0.5;

// Water
uniform float u_evapourationRate;
uniform float u_rainRate;
uniform float u_boilSpeed;
uniform float u_drainRate;
uniform float u_drainMaxDepth;

// Dirt
uniform float u_dirtViscosity;
uniform float u_dirtMaxSlope;
uniform float u_dirtDensity;

// Foam
uniform float u_foamSpawnStrength;
uniform float u_foamDecayRate;

// Erosion
uniform float u_erosionStrength;
uniform float u_erosionMaxDepth;
uniform float u_erosionWaterDepthMin;
uniform float u_erosionWaterDepthMax;
uniform float u_erosionWaterSpeedMax;

// Dirt transport
uniform float u_dirtTransportSpeed;
uniform float u_dirtPickupMinWaterSpeed;
uniform float u_dirtPickupRate;
uniform float u_dirtDepositSpeed;
uniform float u_dissolvedDirtSmoothing;

const float DT = 1.0 / 6.0;

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

layout( location = 0 ) out vec4 out_heightData;
layout( location = 1 ) out vec4 out_miscData;
layout( location = 2 ) out vec4 out_smudgeData;
layout( location = 3 ) out vec4 out_uvOffsetData;

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////
void srand(float seed);
float rand();

////////////////////////////////////////////////////////////////
//
vec2 GetMousePos()
{
	vec2 mousePos = vec2( mouseBufferU, mouseBufferV ) / 255;
	return mousePos;
}

////////////////////////////////////////////////////////////////
//
float exchangeDirt( vec4 _heightDataC, vec4 _heightDataN, float _scalar )
{
	float diff = max( (_heightDataC.x + _heightDataC.y + _heightDataC.z) - (_heightDataN.x + _heightDataN.y + _heightDataN.z), 0.0 );
	float slopeScalar = smoothstep( u_dirtMaxSlope * 0.8, u_dirtMaxSlope, diff );
	diff = min( _heightDataC.z, diff );
	return diff * _scalar * u_dirtViscosity * slopeScalar;
}

////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////
void main(void)
{ 
	ivec2 S = textureSize(s_heightData, 0);

	float cellSize = 1.0 / float(S.x);

	ivec2 T = ivec2(gl_FragCoord.xy);
	srand( T.y * S.x + T.x );
	
	vec4 hC = texelFetch(s_heightData, T, 0);
	
	float solidHeight = hC.x;
	float moltenHeight = hC.y;
	float dirtHeight = hC.z;
	float waterHeight = hC.w;
	
	vec4 miscC = texelFetch(s_miscData, T, 0);
	float heatC = miscC.x;

	vec4 smudgeDataC = texelFetch(s_smudgeData, T, 0);
	vec4 moltenFlux = texelFetch(s_moltenFluxData, T, 0);
	vec2 moltenVelocity = vec2(moltenFlux.y-moltenFlux.x, moltenFlux.w-moltenFlux.z) * 100.0;
	moltenVelocity *= cellSize;
	//float moltenVelocityLength = length(moltenVelocity);
	//moltenVelocity *=  min( 1.0, cellSize * 2.0 / max( moltenVelocityLength, 0.0001 ) );

	////////////////////////////////////////////////////////////////
	// Melt/condense rock
	////////////////////////////////////////////////////////////////
	{
		float condenseRatio = 1.0 - min( heatC / u_moltenMinHeat, 1.0 );
		float condenseSpeed = condenseRatio * u_condenseSpeed;
		
		float condensedAmount = condenseSpeed * hC.y;
		moltenHeight -= condensedAmount;
		solidHeight += condensedAmount;
		
		float heatRatio = max( heatC  - u_moltenMinHeat, 0.0 ) / ( 1.0 - u_moltenMinHeat );

		float meltAmount = min( heatRatio * u_meltSpeed * 0.01, hC.x );
		moltenHeight += meltAmount;
		solidHeight -= meltAmount;
	}

	// Diffuse heat
	{
		float heatU = texelFetchOffset( s_miscData, T, 0, ivec2( 0,-1) ).x;
		float heatD = texelFetchOffset( s_miscData, T, 0, ivec2( 0, 1) ).x;
		float heatR = texelFetchOffset( s_miscData, T, 0, ivec2(-1 ,0) ).x;
		float heatL = texelFetchOffset( s_miscData, T, 0, ivec2( 1, 0) ).x;

		vec4 heatC2 = vec4( heatC );
		vec4 heatN = vec4( heatU, heatD, heatR, heatL );
		vec4 diffs = heatN - heatC2;
		
		vec4 clampedDiffs = clamp( diffs * 0.25 * u_moltenDiffuseStrength, -heatC2, heatN * 0.25 );
		heatC += (clampedDiffs.x + clampedDiffs.y + clampedDiffs.z + clampedDiffs.w) * DT;
	}
	
	////////////////////////////////////////////////////////////////
	// Cooling
	////////////////////////////////////////////////////////////////
	{
		float coolingRatio = 1.0 - mix( 0.0, 0.9, texelFetch(s_derivedData, T, 1).x );
		heatC += (u_ambientTemp - heatC) * u_tempChangeSpeed * coolingRatio;

		// Cool to zero when no molten
		float volumeRatio = min( 1.0, hC.y / 0.0001 );
		heatC -= (1.0-volumeRatio) * heatC * 0.01;
	}
	


	//////////////////////////////////////////////////////////////////////////////////
	// UV OFFSETS
	//////////////////////////////////////////////////////////////////////////////////
	{
		vec4 uvOffsetDataC = texelFetch( s_uvOffsetData, ivec2(gl_FragCoord.xy), 0 );
		
		// Molten
		{
			vec2 uvOffsetA = uvOffsetDataC.xy;
			vec2 uvOffsetB = uvOffsetDataC.zw;
			if ( u_phaseALatch )
			{
				uvOffsetA *= 0.0;
			}
			if ( u_phaseBLatch )
			{
				uvOffsetB *= 0.0;
			}
			
			uvOffsetA += moltenVelocity;
			uvOffsetB += moltenVelocity;
			
			uvOffsetDataC = vec4( uvOffsetA, uvOffsetB );
		}
		
		out_uvOffsetData = uvOffsetDataC;
	}

	////////////////////////////////////////////////////////////////
	// Update dirt
	////////////////////////////////////////////////////////////////
	/*
	{
		float dirtHeightC =  out_heightData.z;
		
		// Transmit some of this cell's volume to neighbours
		float toU = exchangeDirt( heightDataC, heightDataU, 0.19 );
		float toD = exchangeDirt( heightDataC, heightDataD, 0.19 );
		float toL = exchangeDirt( heightDataC, heightDataL, 0.19 );
		float toR = exchangeDirt( heightDataC, heightDataR, 0.19 );
		float toTL = exchangeDirt( heightDataC, heightDataTL, 0.05 );
		float toTR = exchangeDirt( heightDataC, heightDataTR, 0.05 );
		float toBL = exchangeDirt( heightDataC, heightDataBL, 0.05 );
		float toBR = exchangeDirt( heightDataC, heightDataBR, 0.05 );

		float totalOut = toL + toR + toU + toD + toTL + toTR + toBL + toBR;

		// Bring some volume from neighbours
		float fromL = exchangeDirt( heightDataL, heightDataC, 0.19 );
		float fromR = exchangeDirt( heightDataR, heightDataC, 0.19 );
		float fromU = exchangeDirt( heightDataU, heightDataC, 0.19 );
		float fromD = exchangeDirt( heightDataD, heightDataC, 0.19 );
		float fromTL = exchangeDirt( heightDataTL, heightDataC, 0.05 );
		float fromTR = exchangeDirt( heightDataTR, heightDataC, 0.05 );
		float fromBL = exchangeDirt( heightDataBL, heightDataC, 0.05 );
		float fromBR = exchangeDirt( heightDataBR, heightDataC, 0.05 );

		float totalIn = fromL + fromR + fromU + fromD + fromTL + fromTR + fromBL + fromBR;

		dirtHeightC -= totalOut;
		dirtHeightC += totalIn;

		out_heightData.z = dirtHeightC;
	}
	*/
	
	////////////////////////////////////////////////////////////////
	// Water environment
	////////////////////////////////////////////////////////////////
	{
		// Rain / Evapouration / Drainage
		waterHeight -= min( waterHeight / u_drainMaxDepth, 1.0 ) * u_drainRate;
		waterHeight += u_rainRate;
		waterHeight -= u_evapourationRate;
		waterHeight = max(waterHeight, 0.0);

		// Boil from heat
		float waterToBoilOff = min(waterHeight, heatC * u_boilSpeed * 0.01);
		waterHeight -= waterToBoilOff;
		
		// Cool molten based on water boiling
		heatC -= min( heatC, (waterToBoilOff / (1.0+moltenHeight)) * 2000.0 );
		heatC = max(0.0, heatC);
	}	
	
	////////////////////////////////////////////////////////////////
	// Erosion
	////////////////////////////////////////////////////////////////
	/*
	{
		float solidHeight = out_heightData.x;
		float dirtHeight = out_heightData.z;
		float waterHeight = out_heightData.w;
		vec2 waterVelocity = out_velocityData.zw;
		float waterSpeed = length(waterVelocity);

		// Only erode up to a certain depth
		float erosionDirtDepthScalar = 1.0 - smoothstep( 0.0, u_erosionMaxDepth, dirtHeight );

		// Erode more as depth increases, up to a limit
		float erosionWaterDepthScalar = smoothstep( 0.0, u_erosionWaterDepthMax, waterHeight );
		
		// Erode more as speed increases, up to a limit
		float erosionWaterSpeedScalar = smoothstep( 0.0, u_erosionWaterSpeedMax, waterSpeed );

		float rockToDirt = min( erosionWaterSpeedScalar * u_erosionStrength * erosionWaterDepthScalar, solidHeight );

		solidHeight -= rockToDirt;
		dirtHeight += (rockToDirt / u_dirtDensity);
		
		out_heightData.x = solidHeight;
		out_heightData.z = dirtHeight;
	}
	*/
	
	////////////////////////////////////////////////////////////////
	// Deposit some of the dissolved dirt
	////////////////////////////////////////////////////////////////
	/*
	{
		float dissolvedDirt = miscDataC.z;
		float dirtHeight = out_heightData.z;

		float amountDeposited = dissolvedDirt * u_dirtDepositSpeed;
		
		dirtHeight += amountDeposited;
		dissolvedDirt -= amountDeposited;
		dissolvedDirt = max(0.0,dissolvedDirt);

		out_miscData.z = dissolvedDirt;
		out_heightData.z = dirtHeight;
	}

	////////////////////////////////////////////////////////////////
	// Pickup dirt and dissolve it in water
	////////////////////////////////////////////////////////////////
	{
		float dissolvedDirt = miscDataC.z;
		float dirtHeight = out_heightData.z;

		vec2 waterVelocity = out_velocityData.zw;
		float waterSpeed = length(waterVelocity);

		float amountPickedUp = smoothstep( 0.0, u_dirtPickupMinWaterSpeed, waterSpeed ) * u_dirtPickupRate;
		amountPickedUp = min( amountPickedUp, dirtHeight );
		
		dirtHeight -= amountPickedUp;
		dissolvedDirt += amountPickedUp;
		
		out_miscData.z = dissolvedDirt;
		out_heightData.z = dirtHeight;
	}
	*/

	////////////////////////////////////////////////////////////////
	// Move dissolved dirt between cells
	////////////////////////////////////////////////////////////////
	/*
	{
		float dissolvedDirt = out_miscData.z;
		vec2 waterVelocity = out_velocityData.zw;

		float transferedAwayX = min( abs(waterVelocity.x) * u_dirtTransportSpeed, dissolvedDirt * 0.25 );
		float transferedAwayY = min( abs(waterVelocity.y) * u_dirtTransportSpeed, dissolvedDirt * 0.25 );
		float transferedAway = transferedAwayX + transferedAwayY;

		float dissolvedDirtL = miscDataL.z;
		float dissolvedDirtR = miscDataR.z;
		float dissolvedDirtU = miscDataU.z;
		float dissolvedDirtD = miscDataD.z;

		vec2 waterVelocityL = texelFetchL(s_velocityData).zw;
		vec2 waterVelocityR = texelFetchR(s_velocityData).zw;
		vec2 waterVelocityU = texelFetchU(s_velocityData).zw;
		vec2 waterVelocityD = texelFetchD(s_velocityData).zw;

		float transferedInL = min( max( waterVelocityL.x,0.0) * u_dirtTransportSpeed, dissolvedDirtL * 0.25 );
		float transferedInR = min( max(-waterVelocityR.x,0.0) * u_dirtTransportSpeed, dissolvedDirtR * 0.25 );
		float transferedInU = min( max( waterVelocityU.y,0.0) * u_dirtTransportSpeed, dissolvedDirtU * 0.25 );
		float transferedInD = min( max(-waterVelocityD.y,0.0) * u_dirtTransportSpeed, dissolvedDirtD * 0.25 );

		float transferedIn = transferedInL + transferedInR + transferedInU + transferedInD;

		dissolvedDirt += transferedIn;
		dissolvedDirt -= transferedAway;
		dissolvedDirt = max(0.0, dissolvedDirt);

		// Diffuse the dissolved dirt by lerping it towards it's next highest mip. Cheap, but effective. Not volume preserving however.
		dissolvedDirt = mix( dissolvedDirt, textureLod(s_miscData, in_uv, 1).z, u_dissolvedDirtSmoothing );

		out_miscData.z = dissolvedDirt;
	}
	*/

	//////////////////////////////////////////////////////////////////////////////////
	// SMUDGE MAP
	//////////////////////////////////////////////////////////////////////////////////
	{
		vec4 flux = texelFetch(s_moltenFluxData, T, 0);
		vec2 velocity = vec2(flux.y-flux.x, flux.w-flux.z);
		
		float dp = dot( normalize(velocity), normalize(smudgeDataC.xy) );
		if ( isnan(dp) )
			dp = -1.0;

		float ratio = (dp + 1.0) * 0.5;
		smudgeDataC.xy += velocity * u_smudgeChangeRate * 10.0;

		float len = length(smudgeDataC.xy);
		smudgeDataC.xy /= max( 1.0, len );	// Keep smudge vector length less than 1.0

		float pressure = texelFetch(s_pressureData, T, 0).x * 1000;
		
		if ( pressure < 0.0 )
		{
			smudgeDataC.w += pressure * 0.04;
		}
		else
		{
			smudgeDataC.w += pressure * 0.2;
		}
		smudgeDataC.w = clamp( smudgeDataC.w, -1.0, 1.0 );
	}
	
	////////////////////////////////////////////////////////////////
	// Input
	////////////////////////////////////////////////////////////////
	{
		vec4 noiseSample = texture( s_noiseMap, (in_uv / u_mouseRadius) * 0.1 + u_phase * vec2(0.01,0.0) );
		float noise = 1.0;//noiseSample.a;

		vec2 mousePos = GetMousePos();
		float mouseScalar = 1.0f - min(1.0f, length(in_uv-mousePos) / u_mouseRadius);
		mouseScalar *= 0.5;
		
		heatC += pow( mouseScalar, 0.7 ) * u_mouseMoltenHeatStrength * mix( 0.01, 1.0, pow( noise, 4.0 ) );
		heatC = min(1.0, heatC);

		float moltenAddition = mouseScalar * u_mouseMoltenVolumeStrength * mix( 0.01, 1.0, pow( noise, 4.0 ) );
		moltenHeight += moltenAddition;
		//solidHeight += mouseScalar * u_mouseMoltenVolumeStrength * mix( 0.1, 0.0, noise );

		waterHeight += mouseScalar * u_mouseWaterVolumeStrength;
		solidHeight += mouseScalar * u_mouseDirtVolumeStrength;

		// Molten scalar
		miscC.y = mix( miscC.y, pow(noise, 8.0), pow( mouseScalar * u_mouseMoltenHeatStrength * u_mouseMoltenVolumeStrength, 0.5 ) );

		/*
		{
			float dirtHeight = out_heightData.z;

			dirtHeight += pow(mouseRatio, 0.7) * u_mouseDirtVolumeStrength;
			out_heightData.z = dirtHeight;
		}
		*/
	}
		
	out_heightData = max( vec4(0.0), vec4( solidHeight, moltenHeight, dirtHeight, waterHeight ) );
	
	miscC.x = heatC;
	out_miscData = miscC;
	
	out_smudgeData = smudgeDataC;
}