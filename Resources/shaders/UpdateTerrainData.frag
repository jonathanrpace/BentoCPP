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
uniform sampler2D s_velocityData;
uniform sampler2D s_miscData;
uniform sampler2D s_normalData;
uniform sampler2D s_smudgeData;
uniform sampler2D s_waterFluxData;
uniform sampler2D s_grungeMap;


// Mouse
uniform vec2 u_mousePos;
uniform float u_mouseRadius;
uniform float u_mouseMoltenVolumeStrength;
uniform float u_mouseWaterVolumeStrength;
uniform float u_mouseMoltenHeatStrength;
uniform float u_mouseDirtVolumeStrength;

// Environment
uniform float u_ambientTemp;

// Global
uniform float u_heightOffset;

// Molten
uniform float u_heatAdvectSpeed;
uniform float u_moltenViscosity;
uniform float u_rockMeltingPoint;
uniform float u_tempChangeSpeed;
uniform float u_condenseSpeed;
uniform float u_meltSpeed;
uniform float u_moltenVelocityScalar;
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

// Foam
uniform float u_foamSpawnStrength;
uniform float u_foamDecayRate;

// Erosion
uniform float u_erosionStrength;
uniform float u_erosionMaxDepth;
uniform float u_erosionWaterDepthMin;
uniform float u_erosionWaterDepthMax;
uniform float u_erosionWaterSpeedMax;
uniform float u_rockToDirtRatio = 4.0;

// Dirt transport
uniform float u_dirtTransportSpeed;
uniform float u_dirtPickupMinWaterSpeed;
uniform float u_dirtPickupRate;
uniform float u_dirtDepositSpeed;
uniform float u_dissolvedDirtSmoothing;

// Misc
uniform vec2 u_cellSize;
uniform int u_numHeightMips;


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
layout( location = 1 ) out vec4 out_velocityData;
layout( location = 2 ) out vec4 out_miscData;
layout( location = 3 ) out vec4 out_normalData;
layout( location = 4 ) out vec4 out_smudgeData;

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
//
vec2 GetMousePos()
{
	vec2 mousePos = vec2( mouseBufferU, mouseBufferV ) / 255;
	return mousePos;
}

////////////////////////////////////////////////////////////////
//
float CalcMoltenViscosity( float _heat, float _height )
{
	float heightScalar = pow( min(_height / 0.01, 1.0), 0.2 );
	float viscosity = pow( clamp(_heat-u_rockMeltingPoint, 0.0f, 1.0), 0.5 );
	return viscosity * u_moltenViscosity * heightScalar;
}

////////////////////////////////////////////////////////////////
//
vec4 CalcMoltenViscosity( vec4 _heat, vec4 _height )
{
	vec4 heightScalar = pow( min(_height / vec4(0.01), vec4(1.0)), vec4(0.2) );
	vec4 viscosity = pow( clamp(_heat-vec4(u_rockMeltingPoint), vec4(0.0f), vec4(1.0)), vec4(0.5) );
	return viscosity * u_moltenViscosity * heightScalar;
}

////////////////////////////////////////////////////////////////
//
vec2 VelocityFromFlux( vec4 fluxC, vec4 fluxL, vec4 fluxR, vec4 fluxU, vec4 fluxD, float viscosity )
{
	return vec2((fluxL.y + fluxC.y) - (fluxR.x + fluxC.x), 
				(fluxU.w + fluxC.w) - (fluxD.z + fluxC.z) ) * viscosity;
}

////////////////////////////////////////////////////////////////
//
vec4 texelFetchC( sampler2D _sampler ) {
	return texelFetch( _sampler, ivec2(gl_FragCoord.xy), 0 );
}
vec4 texelFetchL( sampler2D _sampler ) {
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(-1,0));
}
vec4 texelFetchR( sampler2D _sampler ) {
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(1,0));
}
vec4 texelFetchU( sampler2D _sampler ) {
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(0,-1));
}
vec4 texelFetchD( sampler2D _sampler ) {
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(0,1));
}
vec4 texelFetchTL( sampler2D _sampler ) {
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(-1,-1));
}
vec4 texelFetchTR( sampler2D _sampler ) {
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(1,-1));
}
vec4 texelFetchBL( sampler2D _sampler ) {
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(-1,1));
}
vec4 texelFetchBR( sampler2D _sampler ) {
	return texelFetchOffset( _sampler, ivec2(gl_FragCoord.xy), 0, ivec2(1,1));
}

float exchange( vec4 _heightDataC, vec4 _heightDataN, vec4 _miscDataC, vec4 _miscDataN, float _scalar )
{
	float viscosity = mix( CalcMoltenViscosity(_miscDataC.x, _heightDataC.y), CalcMoltenViscosity(_miscDataN.x, _heightDataN.y), 0.5 );
	float diff = max( (_heightDataC.x + _heightDataC.y) - (_heightDataN.x + _heightDataN.y), 0.0 );
	diff = min( _heightDataC.y, diff * _scalar * viscosity );
	return diff;
}

float exchangeHeat( float _volumeFromN, vec4 _heightDataN, vec4 _miscDataN, float _limit )
{
	float advectSpeed = mix( u_heatAdvectSpeed * 100.0, u_heatAdvectSpeed, clamp( _volumeFromN / 0.1, 0.0, 1.0 ) );
	return min( _volumeFromN / max(_heightDataN.y, 0.04), _limit) * _miscDataN.x * u_heatAdvectSpeed;
}

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
	// Shared data samples
	vec4 heightDataC = texelFetchC(s_heightData);
	vec4 heightDataL = texelFetchL(s_heightData);
	vec4 heightDataR = texelFetchR(s_heightData);
	vec4 heightDataU = texelFetchU(s_heightData);
	vec4 heightDataD = texelFetchD(s_heightData);
	vec4 heightDataTL = texelFetchTL(s_heightData);
	vec4 heightDataTR = texelFetchTR(s_heightData);
	vec4 heightDataBL = texelFetchBL(s_heightData);
	vec4 heightDataBR = texelFetchBR(s_heightData);

	vec4 miscDataC = texelFetchC(s_miscData);
	vec4 miscDataL = texelFetchL(s_miscData);
	vec4 miscDataR = texelFetchR(s_miscData);
	vec4 miscDataU = texelFetchU(s_miscData);
	vec4 miscDataD = texelFetchD(s_miscData);
	vec4 miscDataTL = texelFetchTL(s_miscData);
	vec4 miscDataTR = texelFetchTR(s_miscData);
	vec4 miscDataBL = texelFetchBL(s_miscData);
	vec4 miscDataBR = texelFetchBR(s_miscData);

	vec4 normalDataC = texelFetchC(s_normalData);
	vec4 velocityDataC = texelFetchC(s_velocityData);
	vec4 smudgeDataC = texelFetchC(s_smudgeData);
	
	vec2 mousePos = GetMousePos();
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-mousePos) / u_mouseRadius);

	// Assign something sensible to ouputs
	// We'll be updating these ahead
	out_heightData = heightDataC;
	out_velocityData = velocityDataC;
	out_miscData = miscDataC;
	out_normalData = normalDataC;
	out_smudgeData = smudgeDataC;

	////////////////////////////////////////////////////////////////
	// Update molten
	////////////////////////////////////////////////////////////////
	{
		float heatC =  miscDataC.x;
		float heightC =  heightDataC.y;
		
		// Transmit some of this cell's volume to neighbours
		float toL = exchange( heightDataC, heightDataL, miscDataC, miscDataL, 0.19 );
		float toR = exchange( heightDataC, heightDataR, miscDataC, miscDataR, 0.19 );
		float toU = exchange( heightDataC, heightDataU, miscDataC, miscDataU, 0.19 );
		float toD = exchange( heightDataC, heightDataD, miscDataC, miscDataD, 0.19 );
		float toTL = exchange( heightDataC, heightDataTL, miscDataC, miscDataTL, 0.05 );
		float toTR = exchange( heightDataC, heightDataTR, miscDataC, miscDataTR, 0.05 );
		float toBL = exchange( heightDataC, heightDataBL, miscDataC, miscDataBL, 0.05 );
		float toBR = exchange( heightDataC, heightDataBR, miscDataC, miscDataBR, 0.05 );

		float totalOut = toL + toR + toU + toD + toTL + toTR + toBL + toBR;



		// Bring some volume from neighbours
		float fromL = exchange( heightDataL, heightDataC, miscDataL, miscDataC, 0.19 );
		float fromR = exchange( heightDataR, heightDataC, miscDataR, miscDataC, 0.19 );
		float fromU = exchange( heightDataU, heightDataC, miscDataU, miscDataC, 0.19 );
		float fromD = exchange( heightDataD, heightDataC, miscDataD, miscDataC, 0.19 );
		float fromTL = exchange( heightDataTL, heightDataC, miscDataTL, miscDataC, 0.05 );
		float fromTR = exchange( heightDataTR, heightDataC, miscDataTR, miscDataC, 0.05 );
		float fromBL = exchange( heightDataBL, heightDataC, miscDataBL, miscDataC, 0.05 );
		float fromBR = exchange( heightDataBR, heightDataC, miscDataBR, miscDataC, 0.05 );

		float totalIn = fromL + fromR + fromU + fromD + fromTL + fromTR + fromBL + fromBR;

		heightC -= totalOut;
		heightC += totalIn;


		// Heat transfer
		float heatLossed = 0.0;
		heatLossed += exchangeHeat( toL, heightDataC, miscDataC, 0.19 );
		heatLossed += exchangeHeat( toR, heightDataC, miscDataC, 0.19 );
		heatLossed += exchangeHeat( toU, heightDataC, miscDataC, 0.19 );
		heatLossed += exchangeHeat( toD, heightDataC, miscDataC, 0.19 );
		heatLossed += exchangeHeat( toTL, heightDataC, miscDataC, 0.05 );
		heatLossed += exchangeHeat( toTR, heightDataC, miscDataC, 0.05 );
		heatLossed += exchangeHeat( toBL, heightDataC, miscDataC, 0.05 );
		heatLossed += exchangeHeat( toBR, heightDataC, miscDataC, 0.05 );


		float heatGained = 0.0;
		heatGained += exchangeHeat( fromL, heightDataL, miscDataL, 0.19 );
		heatGained += exchangeHeat( fromR, heightDataR, miscDataR, 0.19 );
		heatGained += exchangeHeat( fromU, heightDataU, miscDataU, 0.19 );
		heatGained += exchangeHeat( fromD, heightDataD, miscDataD, 0.19 );
		heatGained += exchangeHeat( fromTL, heightDataTL, miscDataTL, 0.05 );
		heatGained += exchangeHeat( fromTR, heightDataTR, miscDataTR, 0.05 );
		heatGained += exchangeHeat( fromBL, heightDataBL, miscDataBL, 0.05 );
		heatGained += exchangeHeat( fromBR, heightDataBR, miscDataBR, 0.05 );

		heatC -= heatLossed;
		heatC += heatGained;

		float totalHeatBefore = (miscDataL.x + miscDataR.x + miscDataU.x + miscDataD.x) * 0.19 + (miscDataTL.x + miscDataTR.x + miscDataBL.x + miscDataBR.x) * 0.05 + miscDataC.x;
		heatC = clamp( min(heatC, totalHeatBefore), 0.0, 2.0 );


		vec2 moltenVelocity = vec2(0.0);
		float diagScalar = normalize( vec2(1.0, 1.0) ).x;
		
		moltenVelocity.x += (toR - toL);
		moltenVelocity.x += (fromL - fromR);
		moltenVelocity.y += (toD - toU);
		moltenVelocity.y += (fromU - fromD);

		moltenVelocity.x += toTR * diagScalar;
		moltenVelocity.x += toBR * diagScalar;
		moltenVelocity.x -= fromTR * diagScalar;
		moltenVelocity.x -= fromBR * diagScalar;
		moltenVelocity.x -= toTL * diagScalar;
		moltenVelocity.x -= toBL * diagScalar;
		moltenVelocity.x += fromTL * diagScalar;
		moltenVelocity.x += fromBL * diagScalar;

		moltenVelocity.y -= toTR * diagScalar;
		moltenVelocity.y += toBR * diagScalar;
		moltenVelocity.y += fromTR * diagScalar;
		moltenVelocity.y -= fromBR * diagScalar;
		moltenVelocity.y -= toTL * diagScalar;
		moltenVelocity.y += toBL * diagScalar;
		moltenVelocity.y += fromTL * diagScalar;
		moltenVelocity.y -= fromBL * diagScalar;
		
		// Cooling
		heatC += (u_ambientTemp - heatC) * u_tempChangeSpeed;

		// Add a bit of smoke for general heat
		out_smudgeData.z += pow(max( heatC-u_rockMeltingPoint, 0.0), 2.0) * 0.03;
		out_smudgeData.z *= 0.99;

		// Add some lava near the mouse
		vec4 diffuseSampleC = texture(s_grungeMap, in_uv+mousePos*0.1);
		float heatTextureScalar = pow( diffuseSampleC.x, 1.0 );
		float heightTextureScalar = diffuseSampleC.x;
		heatC   += ( pow(mouseRatio, 0.7) * u_mouseMoltenHeatStrength   * mix(0.02, 0.02, heatTextureScalar) );
		heightC += ( pow(mouseRatio, 2.0) * u_mouseMoltenVolumeStrength * mix(0.18, 0.2, heightTextureScalar) );
		heatC = max(0.0, heatC);

		out_heightData.y = heightC;
		out_miscData.x = heatC;

		//////////////////////////////////////////////////////////////////////////////////
		// MOLTEN MAPPING
		//////////////////////////////////////////////////////////////////////////////////
		{
			vec2 velocity = velocityDataC.xy;
			out_velocityData.xy = moltenVelocity * u_moltenVelocityScalar;

			vec2 smudgeDir = smudgeDataC.xy;

			smudgeDir += velocity * u_smudgeChangeRate;
			float smudgeLength = sqrt( dot(smudgeDir, smudgeDir) + 0.00001 );
			smudgeDir *= min( 1.0, (1.0/smudgeLength) );

			out_smudgeData.xy = smudgeDir;
		}
	}

	////////////////////////////////////////////////////////////////
	// Add some dirt at mouse position
	////////////////////////////////////////////////////////////////
	{
		float dirtHeight = out_heightData.z;

		dirtHeight += pow(mouseRatio, 0.7) * u_mouseDirtVolumeStrength;

		out_heightData.z = dirtHeight;
	}

	////////////////////////////////////////////////////////////////
	// Update dirt
	////////////////////////////////////////////////////////////////
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

	////////////////////////////////////////////////////////////////
	// Update water height and velocity
	////////////////////////////////////////////////////////////////
	{
		float waterHeight = out_heightData.w;
		
		vec4 fluxC = texelFetchC(s_waterFluxData);
		vec4 fluxL = texelFetchL(s_waterFluxData);
		vec4 fluxR = texelFetchR(s_waterFluxData);
		vec4 fluxU = texelFetchU(s_waterFluxData);
		vec4 fluxD = texelFetchD(s_waterFluxData);
		vec4 fluxN = vec4(fluxL.y, fluxR.x, fluxU.w, fluxD.z);

		float fluxChange = ((fluxN.x+fluxN.y+fluxN.z+fluxN.w)-(fluxC.x+fluxC.y+fluxC.z+fluxC.w));
		waterHeight += fluxChange * u_waterViscosity;

		out_heightData.w = waterHeight;

		////////////////////////////////////////////////////////////////
		// Update water velocity
		////////////////////////////////////////////////////////////////
		vec2 waterVelocity = out_velocityData.zw;

		waterVelocity += VelocityFromFlux( fluxC, fluxL, fluxR, fluxU, fluxD, u_waterViscosity ) * u_waterVelocityScalar;
		waterVelocity *= u_waterVelocityDamping;

		out_velocityData.zw = waterVelocity;
	}

	////////////////////////////////////////////////////////////////
	// Add some water near the mouse
	////////////////////////////////////////////////////////////////
	{
		float waterHeight = out_heightData.w;

		vec4 diffuseSampleC = texture(s_grungeMap, in_uv*2.0+mousePos*0.1);
		float waterMouseScalar = mix( 1.0, pow( diffuseSampleC.x, 1.0 ), 0.2 );
		waterHeight += pow(mouseRatio, 1.1) * u_mouseWaterVolumeStrength * waterMouseScalar;

		out_heightData.w = waterHeight;
	}

	////////////////////////////////////////////////////////////////
	// Water environment
	////////////////////////////////////////////////////////////////
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
		
	////////////////////////////////////////////////////////////////
	// Erosion
	////////////////////////////////////////////////////////////////
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
		dirtHeight += rockToDirt * u_rockToDirtRatio;
		
		out_heightData.x = solidHeight;
		out_heightData.z = dirtHeight;
	}

	////////////////////////////////////////////////////////////////
	// Deposit some of the dissolved dirt
	////////////////////////////////////////////////////////////////
	{
		float dissolvedDirt = out_miscData.z;
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
		float dissolvedDirt = out_miscData.z;
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
		
	////////////////////////////////////////////////////////////////
	// Move dissolved dirt between cells
	////////////////////////////////////////////////////////////////
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

	////////////////////////////////////////////////////////////////
	// Spawn Foam
	////////////////////////////////////////////////////////////////
	{
		vec2 waterVelocity = out_velocityData.zw;

		float foamAmount = smudgeDataC.w;
		foamAmount *= u_foamDecayRate;
		foamAmount += length( waterVelocity ) * u_foamSpawnStrength;
		foamAmount = min(1.0, foamAmount);

		out_smudgeData.w = foamAmount;
	}

	////////////////////////////////////////////////////////////////
	// Melt/condense rock
	////////////////////////////////////////////////////////////////
	{
		float maxTemp = u_rockMeltingPoint * 5.0;
		float heat = out_miscData.x;
		float moltenHeight = out_heightData.y;
		float dirtHeight = out_heightData.z;

		float solidToMolten = smoothstep(u_rockMeltingPoint, maxTemp, heat) * u_meltSpeed;
		solidToMolten = min(solidToMolten, heightDataC.x);

		float moltenToSolid = (1.0-smoothstep(0.0, 0.5, miscDataC.x)) * u_condenseSpeed;
		moltenToSolid = min(moltenToSolid, moltenHeight);

		float dirtToMolten = min( dirtHeight, max(heat-u_rockMeltingPoint, 0.0) * u_meltSpeed * 100.0 );
		
		out_heightData.x -= solidToMolten;
		out_heightData.x += moltenToSolid;
		
		out_heightData.y -= moltenToSolid;
		out_heightData.y += solidToMolten;
		
		out_heightData.x += dirtToMolten;
		out_heightData.z -= dirtToMolten;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Rock normal
	//////////////////////////////////////////////////////////////////////////////////
	{
		vec2 texelSize = vec2(1.0) / vec2(textureSize(s_heightData, 0));

		vec2 uvC = in_uv;
		vec2 uvL = in_uv - vec2(texelSize.x, 0.0);
		vec2 uvR = in_uv + vec2(0.0, texelSize.y);
		vec2 uvU = in_uv - vec2(0.0, texelSize.y);
		vec2 uvD = in_uv + vec2(0.0, texelSize.y);

		vec4 smudgeDataC = out_smudgeData;
		vec4 smudgeDataL = texture(s_smudgeData, uvL);
		vec4 smudgeDataR = texture(s_smudgeData, uvR);
		vec4 smudgeDataU = texture(s_smudgeData, uvU);
		vec4 smudgeDataD = texture(s_smudgeData, uvD);

		float heightC = heightDataC.x + heightDataC.y + heightDataC.z;
		float heightR = heightDataR.x + heightDataR.y + heightDataR.z;
		float heightL = heightDataL.x + heightDataL.y + heightDataL.z;
		float heightU = heightDataU.x + heightDataU.y + heightDataU.z;
		float heightD = heightDataD.x + heightDataD.y + heightDataD.z;
		
		vec3 va = normalize(vec3(u_cellSize.x, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y));

		va = normalize(vec3(u_cellSize.x, heightR-heightL, 0.0f));
		vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y));
		vec3 rockNormal = -cross(va,vb);

		out_normalData.zw = rockNormal.xz;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Water normal
	//////////////////////////////////////////////////////////////////////////////////
	{
		float heightC = heightDataC.x + heightDataC.y + heightDataC.z + heightDataC.w;
		float heightL = heightDataL.x + heightDataL.y + heightDataL.z + heightDataL.w;
		float heightR = heightDataR.x + heightDataR.y + heightDataR.z + heightDataR.w;
		float heightU = heightDataU.x + heightDataU.y + heightDataU.z + heightDataU.w;
		float heightD = heightDataD.x + heightDataD.y + heightDataD.z + heightDataD.w;

		vec3 va = normalize(vec3(u_cellSize.x*2.0, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y*2.0));

		vec3 waterNormal = -cross(va,vb);
		out_normalData.xy = waterNormal.xz;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Occlusion
	//////////////////////////////////////////////////////////////////////////////////
	{
		float occlusion = 0.0f;
		float heightC = heightDataC.x + heightDataC.y + heightDataC.z + miscDataC.y * u_heightOffset;

		float strength = 1.0;
		float totalStrength = 0.0;
		for ( int i = 1; i < u_numHeightMips; i++ )
		{
			vec4 mippedHeightDataC = textureLod(s_heightData, in_uv, float(i));
			vec4 mippedMiscDataC = textureLod(s_miscData, in_uv, float(i));
			vec4 mippedSmudgeDataC = textureLod(s_smudgeData, in_uv, float(i));

			float mippedHeight = mippedHeightDataC.x + mippedHeightDataC.y + mippedHeightDataC.z + mippedMiscDataC.y * u_heightOffset;
			float diff = max(0.0f, mippedHeight - heightC);
			float ratio = diff / u_cellSize.x;
			float angle = atan(ratio);
			float occlusionFoThisMip = angle / HALF_PI;

			occlusion += occlusionFoThisMip * strength;
			totalStrength += strength;
			strength *= 0.5;
		}
		occlusion /= totalStrength;
		occlusion *= 2.0;
		occlusion = min(1.0, occlusion);

		out_miscData.w = occlusion;
	}
}