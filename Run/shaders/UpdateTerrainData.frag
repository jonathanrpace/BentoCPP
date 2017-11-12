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
uniform sampler2D s_grungeMap;
uniform sampler2D s_pressureData;
uniform sampler2D s_derivedData;

// Mouse
uniform float u_mouseRadius;
uniform float u_mouseMoltenVolumeStrength;
uniform float u_mouseWaterVolumeStrength;
uniform float u_mouseMoltenHeatStrength;
uniform float u_mouseDirtVolumeStrength;

// Environment
uniform float u_ambientTemp;
uniform float u_time;
uniform float u_cellSize;
uniform float u_dt;

// Global
uniform float u_heightOffset;
uniform bool u_phaseALatch;
uniform bool u_phaseBLatch;

// Molten
uniform vec2 u_moltenViscosity;
uniform float u_moltenSlopeStrength;
uniform float u_moltenPressureScale;
uniform float u_moltenFluxDamping;
uniform float u_moltenDiffuseStrength;
uniform float u_tempChangeSpeed;
uniform float u_meltSpeed;
uniform float u_condenseSpeed;
uniform float u_smudgeChangeRate;

// Water
uniform float u_waterViscosity;
uniform float u_waterVelocityScalar;
uniform float u_waterVelocityDamping;
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
layout( location = 1 ) out vec4 out_moltenFluxData;
layout( location = 2 ) out vec4 out_waterFluxData;
layout( location = 3 ) out vec4 out_miscData;
layout( location = 4 ) out vec4 out_smudgeData;
layout( location = 5 ) out vec4 out_uvOffsetData;

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

void srand(float _seed);
float rand();

float packUnorm4x8f( vec4 );
vec4 unpackUnorm4x8f( float );

////////////////////////////////////////////////////////////////
//
vec2 GetMousePos()
{
	vec2 mousePos = vec2( mouseBufferU, mouseBufferV ) / 255;
	return mousePos;
}

////////////////////////////////////////////////////////////////
//
vec4 texelFetchC( sampler2D _sampler ) {
	return texture( _sampler, in_uv );
	return texelFetch( _sampler, ivec2(gl_FragCoord.xy), 0 );

}
vec4 texelFetchL( sampler2D _sampler ) {
	return textureOffset( _sampler, in_uv, ivec2(-1,0));
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(-1,0));
}
vec4 texelFetchR( sampler2D _sampler ) {
	return textureOffset( _sampler, in_uv, ivec2(1,0));
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(1,0));
}
vec4 texelFetchU( sampler2D _sampler ) {
	return textureOffset( _sampler, in_uv, ivec2(0,-1));
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(0,-1));
}
vec4 texelFetchD( sampler2D _sampler ) {
	return textureOffset( _sampler, in_uv, ivec2(0,1));
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(0,1));
}
vec4 texelFetchTL( sampler2D _sampler ) {
	return textureOffset( _sampler, in_uv, ivec2(-1,-1));
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(-1,-1));
}
vec4 texelFetchTR( sampler2D _sampler ) {
	return textureOffset( _sampler, in_uv, ivec2(1,-1));
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(1,-1));
}
vec4 texelFetchBL( sampler2D _sampler ) {
	return textureOffset( _sampler, in_uv, ivec2(-1,1));
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(-1,1));
}
vec4 texelFetchBR( sampler2D _sampler ) {
	return textureOffset( _sampler, in_uv, ivec2(1,1));
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(1,1));
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
	ivec2 T = ivec2(gl_FragCoord.xy);
	srand((T.x + T.x * T.y));
	
	vec4 hC = texelFetchC(s_heightData);
	float solidHeight = hC.x;
	float moltenHeight = hC.y;
	float dirtHeight = hC.z;
	float waterHeight = hC.w;
	
	vec4 hN = texelFetchU(s_heightData);
	vec4 hS = texelFetchD(s_heightData);
	vec4 hE = texelFetchR(s_heightData);
	vec4 hW = texelFetchL(s_heightData);
		
	vec4 moltenFluxC = texelFetchC(s_moltenFluxData);
	vec4 waterFluxC = texelFetchC(s_waterFluxData);
	
	vec4 miscC = texelFetchC(s_miscData);
	float heatC = miscC.x;
	
	vec4 uvOffsetDataC = texelFetchC(s_uvOffsetData);
	vec4 smudgeDataC = texelFetchC(s_smudgeData);
	

	////////////////////////////////////////////////////////////////
	// Update molten heights and heat based on moltenFluxC
	////////////////////////////////////////////////////////////////
	{
		vec4 fluxN = texelFetchU(s_moltenFluxData);
		vec4 fluxS = texelFetchD(s_moltenFluxData);
		vec4 fluxE = texelFetchR(s_moltenFluxData);
		vec4 fluxW = texelFetchL(s_moltenFluxData);
		
		float toN = min( moltenFluxC.z, hC.y ) * DT;
		float toS = min( moltenFluxC.w, hC.y ) * DT;
		float toE = min( moltenFluxC.y, hC.y ) * DT;
		float toW = min( moltenFluxC.x, hC.y ) * DT;
		float totalTo = (toN + toS + toE + toW);

		float fromN = min( fluxN.w, hN.y ) * DT;
		float fromS = min( fluxS.z, hS.y ) * DT;
		float fromE = min( fluxE.x, hE.y ) * DT;
		float fromW = min( fluxW.y, hW.y ) * DT;

		float totalFrom = fromN + fromS + fromE + fromW;
		
		moltenHeight += totalFrom;
		moltenHeight -= totalTo;
	
		if ( hC.y > 0 )
		{
			// Advect heat
			float heatN = texelFetchU(s_miscData).x;
			float heatS = texelFetchD(s_miscData).x;
			float heatE = texelFetchR(s_miscData).x;
			float heatW = texelFetchL(s_miscData).x;
		
			float propC = min( 1.0, max( 0.0, hC.y - totalTo ) / moltenHeight );
			float propN = min( 1.0, fromN / moltenHeight );
			float propS = min( 1.0, fromS / moltenHeight );
			float propE = min( 1.0, fromE / moltenHeight );
			float propW = min( 1.0, fromW / moltenHeight );
			
			// New heat is the average of all the incoming heat, weighted by the proportion of the volume incoming from each direction
			heatC = (propC * heatC) + (propN * heatN) + (propS * heatS) + (propE * heatE) + (propW * heatW);
		}
	}
	
	////////////////////////////////////////////////////////////////
	// Update water height based on waterFluxC
	////////////////////////////////////////////////////////////////
	{
		vec4 fluxN = texelFetchU(s_waterFluxData);
		vec4 fluxS = texelFetchD(s_waterFluxData);
		vec4 fluxE = texelFetchR(s_waterFluxData);
		vec4 fluxW = texelFetchL(s_waterFluxData);
		
		float toN = min( waterFluxC.z, hC.w ) * DT;
		float toS = min( waterFluxC.w, hC.w ) * DT;
		float toE = min( waterFluxC.y, hC.w ) * DT;
		float toW = min( waterFluxC.x, hC.w ) * DT;
		float totalTo = (toN + toS + toE + toW);

		float fromN = min( fluxN.w, hN.w ) * DT;
		float fromS = min( fluxS.z, hS.w ) * DT;
		float fromE = min( fluxE.x, hE.w ) * DT;
		float fromW = min( fluxW.y, hW.w ) * DT;

		float totalFrom = fromN + fromS + fromE + fromW;
		
		waterHeight += totalFrom;
		waterHeight -= totalTo;
	}
	
	////////////////////////////////////////////////////////////////
	// Melt/condense rock
	////////////////////////////////////////////////////////////////
	{
		float condensedAmount = u_condenseSpeed * hC.y * 0.01;
		moltenHeight -= condensedAmount;
		solidHeight += condensedAmount;
		
		float heatRatio = min( miscC.x, 1.0 );
		float meltAmount = heatRatio * u_meltSpeed * hC.x * 0.01;
		moltenHeight += meltAmount;
		solidHeight -= meltAmount;
	}
	
	////////////////////////////////////////////////////////////////
	// Modifications to moltenFlux allowed after this point
	// (Prior to this point we were using the same values our
	// neightbours will be accessing)
	////////////////////////////////////////////////////////////////
	{
		////////////////////////////////////////////////////////////////
		// Diffuse volume between cells using a volume preserving blur
		////////////////////////////////////////////////////////////////
		{
			vec4 heightC = vec4( solidHeight + moltenHeight );
			vec4 heightN = vec4( hW.x + hW.y, hE.x + hE.y, hN.x + hN.y, hS.x + hS.y );
			vec4 diffs = heightN - heightC;
			
			vec4 clampedDiffs = clamp( diffs * 0.25 * u_moltenDiffuseStrength, -vec4(hC.y * 0.25), vec4(hW.y, hE.y, hN.y, hS.y) * 0.25 );
			moltenHeight += (clampedDiffs.x + clampedDiffs.y + clampedDiffs.z + clampedDiffs.w) * DT;
			moltenHeight = max(0.0, moltenHeight);
		}
		
		////////////////////////////////////////////////////////////////
		// Add slope to molten flux
		////////////////////////////////////////////////////////////////
		{
			float mhC = solidHeight + moltenHeight;
			float mhN = hN.x + hN.y;
			float mhS = hS.x + hS.y;
			float mhE = hE.x + hE.y;
			float mhW = hW.x + hW.y;
			vec4 diffs = vec4( mhC - mhW, mhC - mhE, mhC - mhN, mhC - mhS );
			diffs *= u_moltenSlopeStrength;
			
			////////////////////////////////////////////////////////////////
			// Add pressure gradient to diffs
			////////////////////////////////////////////////////////////////
			{
				float pC = texelFetchOffset(s_pressureData, T, 0, ivec2( 0,  0)).x * 10000.0;
				float pN = texelFetchOffset(s_pressureData, T, 0, ivec2( 0, -1)).x * 10000.0;
				float pS = texelFetchOffset(s_pressureData, T, 0, ivec2( 0,  1)).x * 10000.0;
				float pE = texelFetchOffset(s_pressureData, T, 0, ivec2( 1,  0)).x * 10000.0;
				float pW = texelFetchOffset(s_pressureData, T, 0, ivec2(-1,  0)).x * 10000.0;
				
				pC = max( 0.0, pC );
				pN = max( 0.0, pN );
				pS = max( 0.0, pS );
				pE = max( 0.0, pE );
				pW = max( 0.0, pW );
			
				diffs.x += (pC - pW) * u_moltenPressureScale;
				diffs.y += (pC - pE) * u_moltenPressureScale;
				diffs.z += (pC - pN) * u_moltenPressureScale;
				diffs.w += (pC - pS) * u_moltenPressureScale;
			}
			
			moltenFluxC = max( vec4(0.0), moltenFluxC + diffs * DT );
			float scalingFactor = min( 1.0, mhC / (moltenFluxC.x + moltenFluxC.y + moltenFluxC.z + moltenFluxC.w) );
			moltenFluxC *= scalingFactor;
		}
		
		////////////////////////////////////////////////////////////////
		// Damping
		////////////////////////////////////////////////////////////////
		{
			// Global damping
			moltenFluxC -= moltenFluxC * u_moltenFluxDamping;
			
			// Dampen to zero when no volume
			float volumeRatio = min( 1.0, hC.y / 0.001 );
			moltenFluxC *= volumeRatio;
			
			// Dampen based on heat/viscosity
			float moltenViscosity = mix( u_moltenViscosity.x, u_moltenViscosity.y, min( 1.0, miscC.x * 0.5 ) );
			moltenFluxC *= moltenViscosity;
		}
	}
	
	
	////////////////////////////////////////////////////////////////
	// Cooling
	////////////////////////////////////////////////////////////////
	{
		float coolingRatio = 1.0 - mix( 0.0, 0.9, texelFetch(s_derivedData, T, 1).x );
		heatC += (u_ambientTemp - heatC) * u_tempChangeSpeed * coolingRatio;
		
		// Cool more when no volume
		if ( moltenHeight < 0.001 )
		{
			heatC *= 0.995f;
		}
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
	/*
	{
		float waterHeight = out_heightData.w;
		float moltenHeight = out_heightData.y;
		float heat = out_miscData.x;

		// Rain / Evapouration / Drainage
		waterHeight -= min( waterHeight / u_drainMaxDepth, 1.0 ) * u_drainRate;
		waterHeight += u_rainRate;
		waterHeight -= u_evapourationRate;
		waterHeight = max(waterHeight, 0.0);

		// Boil from heat
		float waterToBoilOff = min(waterHeight, heat * u_boilSpeed);
		waterHeight -= waterToBoilOff;
		
		// Cool molten based on water boiling
		heat -= min( heat, (waterToBoilOff / (1.0+moltenHeight)) * 50.0 );

		out_heightData.w = waterHeight;
		out_miscData.x = heat;
	}	
	*/
	
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
	// UV OFFSETS
	//////////////////////////////////////////////////////////////////////////////////
	{
		vec2 velocity = vec2(moltenFluxC.y-moltenFluxC.x, moltenFluxC.w-moltenFluxC.z);
		
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
		
		uvOffsetA += velocity * 1000.0;
		uvOffsetB += velocity * 1000.0;
		
		uvOffsetDataC = vec4( uvOffsetA, uvOffsetB );
	}

	//////////////////////////////////////////////////////////////////////////////////
	// SMUDGE MAP
	//////////////////////////////////////////////////////////////////////////////////
	/*
	{
		vec2 velocity = vec2(fC.y-fC.x, fC.w-fC.z);
		
		float dp = dot( normalize(velocity), normalize(smudgeDataC.xy) );
		if ( isnan(dp) )
			dp = -1.0;

		float ratio = (dp + 1.0) * 0.5;
		smudgeDataC.xy += velocity * u_smudgeChangeRate;
	}
	*/
	
	////////////////////////////////////////////////////////////////
	// Input
	////////////////////////////////////////////////////////////////
	{
		vec2 mousePos = GetMousePos();
		float mouseRatio = 1.0f - min(1.0f, length(in_uv-mousePos) / u_mouseRadius);
		float mouseScalar = pow(mouseRatio, 2.0);
		float volumeStrength = mouseScalar * u_mouseMoltenVolumeStrength;
		heatC += mouseScalar * u_mouseMoltenHeatStrength * mix( 0.5, 1.0, rand() ) * 0.4;
		moltenHeight += mouseScalar * u_mouseMoltenVolumeStrength * 0.5;
		solidHeight += mouseScalar * u_mouseDirtVolumeStrength * 0.5;
		waterHeight += mouseScalar * u_mouseWaterVolumeStrength * 0.5;
		
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
	out_moltenFluxData = moltenFluxC;
	out_waterFluxData = waterFluxC;
	out_uvOffsetData = uvOffsetDataC;
}