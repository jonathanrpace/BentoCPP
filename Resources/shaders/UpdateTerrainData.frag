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
uniform sampler2D s_moltenMapData;
uniform sampler2D s_smudgeData;
uniform sampler2D s_waterFluxData;
uniform sampler2D s_rockFluxData;
uniform sampler2D s_diffuseMap;


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
uniform float u_moltenViscosity;
uniform float u_rockMeltingPoint;
uniform float u_tempChangeSpeed;
uniform float u_condenseSpeed;
uniform float u_meltSpeed;
uniform float u_moltenVelocityScalar;
uniform float u_moltenVelocityDamping;
uniform float u_mapHeightOffset;

// Water
uniform float u_waterViscosity;
uniform float u_waterVelocityScalar;
uniform float u_waterVelocityDamping;
uniform float u_evapourationRate;
uniform float u_rainRate;
uniform float u_boilSpeed;
uniform float u_drainRate;
uniform float u_drainMaxDepth;

// Erosion
uniform float u_erosionStrength;
uniform float u_erosionMaxDepth;
uniform float u_erosionWaterDepthMin;
uniform float u_erosionWaterDepthMax;
uniform float u_erosionWaterSpeedMax;
uniform float u_rockToDirtRatio = 1.5;

// Dirt transport
uniform float u_dirtTransportSpeed;
uniform float u_dirtPickupMinWaterSpeed;
uniform float u_dirtPickupRate;
uniform float u_dirtDepositSpeed;

// Vegetation
uniform float u_vegMinDirt;
uniform float u_vegMaxDirt;
uniform float u_vegMinSlope;
uniform float u_vegMaxSlope;
uniform float u_vegGrowthRate;
uniform float u_vegBump;


// Misc
uniform float u_textureScrollSpeed;
uniform float u_cycleSpeed;
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
float CalcMoltenViscosity( float _heat )
{
	return smoothstep( 0.0f, 1.0f, clamp(_heat-u_rockMeltingPoint, 0.0f, 1.0)) * u_moltenViscosity;
}

////////////////////////////////////////////////////////////////
//
vec4 CalcMoltenViscosity( vec4 _heat )
{
	return smoothstep( vec4(0.0f), vec4(1.0f), clamp(_heat-vec4(u_rockMeltingPoint), vec4(0.0f), vec4(1.0))) * vec4(u_moltenViscosity);
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

	vec4 miscDataC = texelFetchC(s_miscData);
	vec4 miscDataL = texelFetchL(s_miscData);
	vec4 miscDataR = texelFetchR(s_miscData);
	vec4 miscDataU = texelFetchU(s_miscData);
	vec4 miscDataD = texelFetchD(s_miscData);

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
		float heat = miscDataC.x;
		float height = heightDataC.y;

		float viscosity = CalcMoltenViscosity(heat);

		vec4 fluxC = texelFetchC(s_rockFluxData);
		vec4 fluxL = texelFetchL(s_rockFluxData);
		vec4 fluxR = texelFetchR(s_rockFluxData);
		vec4 fluxU = texelFetchU(s_rockFluxData);
		vec4 fluxD = texelFetchD(s_rockFluxData);
		vec4 fluxN = vec4(fluxL.y, fluxR.x, fluxU.w, fluxD.z);

		// Update molten height based on flux
		float fluxChange = ((fluxN.x+fluxN.y+fluxN.z+fluxN.w)-(fluxC.x+fluxC.y+fluxC.z+fluxC.w));
		height += fluxChange * viscosity;
		
		// What proportion of our volume did we lose to neighbours?
		// If we lose half our volume, we also lose half our heat.
		float volumeLossProp = ((fluxC.x+fluxC.y+fluxC.z+fluxC.w) * viscosity) / (height + EPSILON.x);
		volumeLossProp = min( 1.0f, volumeLossProp );
		heat -= (volumeLossProp * heat) * u_heatAdvectSpeed;

		// For each neighbour, determine what proportion of their volume we have gained.
		// We also want to grab the same proportion of their heat.
		// Essentially the inverse of above.
		vec4 heatN = vec4(miscDataL.x, miscDataR.x, miscDataU.x, miscDataD.x);
		vec4 heightN = vec4(heightDataL.y, heightDataR.y, heightDataU.y, heightDataD.y);
		vec4 viscosityN = CalcMoltenViscosity(heatN);
		vec4 volumeGainProp = (max(vec4(0.0), fluxN-fluxC) * viscosityN) / (heightN + EPSILON);
		volumeGainProp = min( vec4(1.0), volumeGainProp );
		vec4 heatGain = volumeGainProp * heatN;
		heat += (heatGain.x + heatGain.y + heatGain.z + heatGain.w) * u_heatAdvectSpeed;

		// Cooling
		// Occluded areas cool slower
		float occlusion = miscDataC.w;
		heat += (u_ambientTemp - heat) * u_tempChangeSpeed;

		// Add some lava near the mouse
		vec4 diffuseSampleC = texture(s_diffuseMap, in_uv+mousePos*0.01);
		float heatTextureScalar = pow( diffuseSampleC.x, 1.0 );
		float heightTextureScalar = mix( 1.0-diffuseSampleC.x, diffuseSampleC.x, 0.75 );
		float heightMin = 1.0 - min( 0.5 + heat * 2.0, 1.0 );
		heat   += ( pow(mouseRatio, 1.0) * u_mouseMoltenHeatStrength   * mix(0.25, 1.0, heatTextureScalar) ) / (1.0+heat*10.0);
		height += ( pow(mouseRatio, 2.0) * u_mouseMoltenVolumeStrength * mix(0.0, 1.0, heightTextureScalar) ) / (1.0+height);

		heat = max(0.0, heat);

		out_heightData.y = height;
		out_miscData.x = heat;

		//////////////////////////////////////////////////////////////////////////////////
		// MOLTEN MAPPING
		//////////////////////////////////////////////////////////////////////////////////
		{
			vec2 velocity = velocityDataC.xy;
			velocity += VelocityFromFlux( fluxC, fluxL, fluxR, fluxU, fluxD, viscosity ) * u_moltenVelocityScalar;
			velocity *= u_moltenVelocityDamping;
			out_velocityData.xy = velocity;

			vec2 smudgeUV = smudgeDataC.xy;

			float smudgeAmount = length(smudgeUV+EPSILON.xy);
			float moltenSpeed = length(velocity+EPSILON.xy);

			float influence = moltenSpeed;// * (1.0 - min( 1.0, smudgeAmount / 0.02 ));
			float dp = clamp( dot( normalize(velocity+EPSILON.xy), normalize(smudgeUV+EPSILON.xy) ), 0.0, 1.0 );
			dp = 1.0 - dp;

			vec2 velocityN = min( velocity / moltenSpeed, vec2(1.0) );
			//smudgeUV += velocityN * influence * 0.01 * dp;

			out_smudgeData.xy = smudgeUV;
		}
	}

	////////////////////////////////////////////////////////////////
	// Update water
	////////////////////////////////////////////////////////////////
	{
		float waterHeight = heightDataC.w;
		
		vec4 fluxC = texelFetchC(s_waterFluxData);
		vec4 fluxL = texelFetchL(s_waterFluxData);
		vec4 fluxR = texelFetchR(s_waterFluxData);
		vec4 fluxU = texelFetchU(s_waterFluxData);
		vec4 fluxD = texelFetchD(s_waterFluxData);
		vec4 fluxN = vec4(fluxL.y, fluxR.x, fluxU.w, fluxD.z);

		// Update water height based on flux
		float fluxChange = ((fluxN.x+fluxN.y+fluxN.z+fluxN.w)-(fluxC.x+fluxC.y+fluxC.z+fluxC.w));
		waterHeight += fluxChange * u_waterViscosity;

		// Add some water near the mouse
		vec4 diffuseSampleC = texture(s_diffuseMap, in_uv*2.0+mousePos*0.1);
		float waterMouseScalar = mix( 1.0, pow( diffuseSampleC.x, 1.0 ), 0.2 );
		waterHeight += pow(mouseRatio, 1.1) * u_mouseWaterVolumeStrength * waterMouseScalar;

		// Drainage
		waterHeight -= min( waterHeight / u_drainMaxDepth, 1.0 ) * u_drainRate;

		waterHeight += u_rainRate;
		waterHeight -= u_evapourationRate;

		waterHeight = max(waterHeight, 0.0);

		// Boil off any water due to heat
		float heat = out_miscData.x;
		float waterToBoilOff = min(waterHeight, max(heat-u_rockMeltingPoint, 0.0) * u_boilSpeed);
		waterHeight -= waterToBoilOff;
		float moltenHeight = out_heightData.y;
		heat -= min( heat, (waterToBoilOff / (1.0+moltenHeight)) * 10.0 );
		out_heightData.w = waterHeight;
		out_miscData.x = heat;
		out_smudgeData.z += waterToBoilOff * 100.0;


		
		/*
		////////////////////////////////////////////////////////////////
		// Erosion
		////////////////////////////////////////////////////////////////

		vec2 waterVelocityC = velocityDataC.zw;
		vec2 waterVelocityL = texelFetchL(s_velocityData).zw;
		vec2 waterVelocityR = texelFetchR(s_velocityData).zw;
		vec2 waterVelocityU = texelFetchU(s_velocityData).zw;
		vec2 waterVelocityD = texelFetchD(s_velocityData).zw;
		vec2 waterVelocityTL = texelFetchTL(s_velocityData).zw;
		vec2 waterVelocityTR = texelFetchTR(s_velocityData).zw;
		vec2 waterVelocityBL = texelFetchBL(s_velocityData).zw;
		vec2 waterVelocityBR = texelFetchBR(s_velocityData).zw;

		float waterSpeedC = length(waterVelocityC) + EPSILON.x;
		float waterSpeedL = length(waterVelocityL) + EPSILON.x;
		float waterSpeedR = length(waterVelocityR) + EPSILON.x;
		float waterSpeedU = length(waterVelocityU) + EPSILON.x;
		float waterSpeedD = length(waterVelocityD) + EPSILON.x;
		float waterSpeedTL = length(waterVelocityTL) + EPSILON.x;
		float waterSpeedTR = length(waterVelocityTR) + EPSILON.x;
		float waterSpeedBL = length(waterVelocityBL) + EPSILON.x;
		float waterSpeedBR = length(waterVelocityBR) + EPSILON.x;

		float waterSpeedScalarC = smoothstep( 0.0, u_erosionWaterSpeedMax, waterSpeedC );
		float waterSpeedScalarL = smoothstep( 0.0, u_erosionWaterSpeedMax, waterSpeedL );
		float waterSpeedScalarR = smoothstep( 0.0, u_erosionWaterSpeedMax, waterSpeedR );
		float waterSpeedScalarU = smoothstep( 0.0, u_erosionWaterSpeedMax, waterSpeedU );
		float waterSpeedScalarD = smoothstep( 0.0, u_erosionWaterSpeedMax, waterSpeedD );
		float waterSpeedScalarTL = smoothstep( 0.0, u_erosionWaterSpeedMax, waterSpeedTL );
		float waterSpeedScalarTR = smoothstep( 0.0, u_erosionWaterSpeedMax, waterSpeedTR );
		float waterSpeedScalarBL = smoothstep( 0.0, u_erosionWaterSpeedMax, waterSpeedBL );
		float waterSpeedScalarBR = smoothstep( 0.0, u_erosionWaterSpeedMax, waterSpeedBR );

		float dirtC = out_heightData.z;
		float dirtL = heightDataL.z;
		float dirtR = heightDataR.z;
		float dirtU = heightDataU.z;
		float dirtD = heightDataD.z;
		float dirtTL = texelFetchTL(s_heightData).z;
		float dirtTR = texelFetchTR(s_heightData).z;
		float dirtBL = texelFetchBL(s_heightData).z;
		float dirtBR = texelFetchBR(s_heightData).z;

		// Only erode rock when there's not much dirt covering it.
		float erosionDepthScalar = 1.0 - min(dirtC / u_erosionMaxDepth, 1.0);
		float solidHeight = out_heightData.x;
		float rockToDirt = min( erosionDepthScalar * waterSpeedScalarC * u_erosionStrength, solidHeight );


		////////////////////////////////////////////////////////////////
		// Dirt transfer
		////////////////////////////////////////////////////////////////
		
		// Transfer dirt from neightbours
		float dpL = clamp( dot( vec2( 1.0,  0.0 ), waterVelocityL / waterSpeedL ), 0.0, 1.0 );
		float dpR = clamp( dot( vec2(-1.0,  0.0 ), waterVelocityR / waterSpeedR ), 0.0, 1.0 );
		float dpU = clamp( dot( vec2( 0.0,  1.0 ), waterVelocityU / waterSpeedU ), 0.0, 1.0 );
		float dpD = clamp( dot( vec2( 0.0, -1.0 ), waterVelocityD / waterSpeedD ), 0.0, 1.0 );
		float dpTL = clamp( dot( vec2( 0.707, 0.707 ), waterVelocityTL / waterSpeedTL ), 0.0, 1.0 );
		float dpTR = clamp( dot( vec2(-0.707,  0.707 ), waterVelocityTR / waterSpeedTR ), 0.0, 1.0 );
		float dpBL = clamp( dot( vec2( 0.707,  -0.707 ), waterVelocityBL / waterSpeedBL ), 0.0, 1.0 );
		float dpBR = clamp( dot( vec2( -0.707, -0.707 ), waterVelocityBR / waterSpeedBR ), 0.0, 1.0 );

		float transferedIn = 0.0;
		transferedIn += waterSpeedScalarL * dpL * u_dirtTransportSpeed * dirtL * 0.25;
		transferedIn += waterSpeedScalarR * dpR * u_dirtTransportSpeed * dirtR * 0.25;
		transferedIn += waterSpeedScalarU * dpU * u_dirtTransportSpeed * dirtU * 0.25;
		transferedIn += waterSpeedScalarD * dpD * u_dirtTransportSpeed * dirtD * 0.25;

		transferedIn += waterSpeedScalarTL * dpTL * u_dirtTransportSpeed * dirtTL * 0.25 * 0.707;
		transferedIn += waterSpeedScalarTR * dpTR * u_dirtTransportSpeed * dirtTR * 0.25 * 0.707;
		transferedIn += waterSpeedScalarBL * dpBL * u_dirtTransportSpeed * dirtBL * 0.25 * 0.707;
		transferedIn += waterSpeedScalarBR * dpBR * u_dirtTransportSpeed * dirtBR * 0.25 * 0.707;

		// Distribute dirt to neigbours
		dpL = clamp( dot( vec2( 1.0,  0.0 ), waterVelocityC / waterSpeedC ), 0.0, 1.0 );
		dpR = clamp( dot( vec2(-1.0,  0.0 ), waterVelocityC / waterSpeedC ), 0.0, 1.0 );
		dpU = clamp( dot( vec2( 0.0,  1.0 ), waterVelocityC / waterSpeedC ), 0.0, 1.0 );
		dpD = clamp( dot( vec2( 0.0, -1.0 ), waterVelocityC / waterSpeedC ), 0.0, 1.0 );

		dpTL = clamp( dot( vec2( 0.707, 0.707 ), waterVelocityC / waterSpeedC ), 0.0, 1.0 );
		dpTR = clamp( dot( vec2(-0.707,  0.707 ), waterVelocityC / waterSpeedC ), 0.0, 1.0 );
		dpBL = clamp( dot( vec2( 0.707,  -0.707 ), waterVelocityC / waterSpeedC ), 0.0, 1.0 );
		dpBR = clamp( dot( vec2( -0.707, -0.707 ), waterVelocityC / waterSpeedC ), 0.0, 1.0 );

		float transferedAway = 0.0;
		transferedAway += waterSpeedScalarC * dpL * u_dirtTransportSpeed * dirtC * 0.25;
		transferedAway += waterSpeedScalarC * dpR * u_dirtTransportSpeed * dirtC * 0.25;
		transferedAway += waterSpeedScalarC * dpU * u_dirtTransportSpeed * dirtC * 0.25;
		transferedAway += waterSpeedScalarC * dpD * u_dirtTransportSpeed * dirtC * 0.25;

		transferedAway += waterSpeedScalarC * dpTL * u_dirtTransportSpeed * dirtC * 0.25 * 0.707;
		transferedAway += waterSpeedScalarC * dpTR * u_dirtTransportSpeed * dirtC * 0.25 * 0.707;
		transferedAway += waterSpeedScalarC * dpBL * u_dirtTransportSpeed * dirtC * 0.25 * 0.707;
		transferedAway += waterSpeedScalarC * dpBR * u_dirtTransportSpeed * dirtC * 0.25 * 0.707;


		out_heightData.z = dirtC + rockToDirt + transferedIn - transferedAway;

		out_heightData.x -= rockToDirt;
		*/


		
		// Only erode up to a certain depth
		float dirtHeight = out_heightData.z;
		float erosionDirtDepthScalar = 1.0 - min(dirtHeight / u_erosionMaxDepth, 1.0);

		// Erode more as water depth increases from zero, up to a point, then diminish.
		float erosionWaterDepthScalar =        smoothstep(0.0,                    u_erosionWaterDepthMin, waterHeight) 
										* (1.0-smoothstep(u_erosionWaterDepthMin, u_erosionWaterDepthMax, waterHeight));

		// Erode more if water is moving fast
		vec2 waterVelocityC = velocityDataC.zw;
		float waterSpeedC = length(waterVelocityC);
		if ( isnan(waterSpeedC) ) waterSpeedC = 0.0;
		if ( isinf(waterSpeedC) ) waterSpeedC = 0.0;
		float erosionWaterSpeedScalar = smoothstep( 0.0, u_erosionWaterSpeedMax, waterSpeedC );

		float solidHeight = out_heightData.x;
		float rockToDirt = min( erosionDirtDepthScalar * erosionWaterDepthScalar * erosionWaterSpeedScalar * u_erosionStrength, solidHeight );
		solidHeight -= rockToDirt;
		dirtHeight += rockToDirt * u_rockToDirtRatio;
		
		out_heightData.x = solidHeight;

		////////////////////////////////////////////////////////////////
		// Deposit some of the dissolved dirt
		////////////////////////////////////////////////////////////////
		float dissolvedDirt = miscDataC.z;
		float amountDeposited = dissolvedDirt * u_dirtDepositSpeed;
		
		dirtHeight += amountDeposited;
		dissolvedDirt -= amountDeposited;
		dissolvedDirt = max(0.0,dissolvedDirt);

		////////////////////////////////////////////////////////////////
		// Pickup dirt and dissolve it in water
		////////////////////////////////////////////////////////////////
		float amountPickedUp = min( max( waterSpeedC-u_dirtPickupMinWaterSpeed, 0.0 ) * u_dirtPickupRate, dirtHeight-amountDeposited );
		
		dirtHeight -= amountPickedUp;
		dissolvedDirt += amountPickedUp;
		dissolvedDirt = max(0.0,dissolvedDirt);

		out_heightData.z = dirtHeight;

		////////////////////////////////////////////////////////////////
		// Move dissolved dirt between cells
		////////////////////////////////////////////////////////////////
		float transferedAwayX = min( abs(waterVelocityC.x) * u_dirtTransportSpeed, dissolvedDirt * 0.25 );
		float transferedAwayY = min( abs(waterVelocityC.y) * u_dirtTransportSpeed, dissolvedDirt * 0.25 );
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

		dissolvedDirt += (transferedIn - transferedAway);
		dissolvedDirt = max(0.0, dissolvedDirt);

		out_miscData.z = dissolvedDirt;
		


		////////////////////////////////////////////////////////////////
		// Update water velocity
		////////////////////////////////////////////////////////////////
		waterVelocityC += VelocityFromFlux( fluxC, fluxL, fluxR, fluxU, fluxD, u_waterViscosity ) * u_waterVelocityScalar;
		waterVelocityC *= u_waterVelocityDamping;

		out_velocityData.zw = waterVelocityC;
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

		float moltenToSolid = (1.0-smoothstep(0, u_rockMeltingPoint, miscDataC.x)) * u_condenseSpeed;
		moltenToSolid = min(moltenToSolid, moltenHeight);

		float dirtToMolten = min( dirtHeight, max(heat-u_rockMeltingPoint, 0.0) * u_meltSpeed * 100.0 );

		out_heightData.x -= solidToMolten;
		out_heightData.x += moltenToSolid;
		
		out_heightData.y -= moltenToSolid;
		out_heightData.y += solidToMolten;

		out_heightData.x += dirtToMolten;
		out_heightData.z -= dirtToMolten;

		out_smudgeData.z += dirtToMolten * 100.0;

		// Dampen the amount of steam
		out_smudgeData.z *= 0.997;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Rock normal
	//////////////////////////////////////////////////////////////////////////////////
	vec3 rockNormal;
	vec3 rockNormalSansVeg;
	{
		vec2 texelSize = vec2(1.0) / vec2(textureSize(s_moltenMapData, 0));

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

		float strength = 1.0;
		float totalStrength = 0.0;
		float moltenMapResultC = 0.0;
		for ( int i = 0; i < 6; i++ )
		{
			moltenMapResultC += textureLod(s_moltenMapData, uvC - smudgeDataC.xy, i).x * strength;

			totalStrength += strength;
			strength *= 1.0;
		}
		moltenMapResultC /= totalStrength;

		float moltenMapResultL = miscDataL.y;
		float moltenMapResultR = miscDataR.y;
		float moltenMapResultU = miscDataU.y;
		float moltenMapResultD = miscDataD.y;

		float heightC = heightDataC.x + heightDataC.y + heightDataC.z + moltenMapResultC * u_mapHeightOffset;
		float heightR = heightDataR.x + heightDataR.y + heightDataR.z + moltenMapResultR * u_mapHeightOffset;
		float heightL = heightDataL.x + heightDataL.y + heightDataL.z + moltenMapResultL * u_mapHeightOffset;
		float heightU = heightDataU.x + heightDataU.y + heightDataU.z + moltenMapResultU * u_mapHeightOffset;
		float heightD = heightDataD.x + heightDataD.y + heightDataD.z + moltenMapResultD * u_mapHeightOffset;
		
		vec3 va = normalize(vec3(u_cellSize.x, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y));
		rockNormalSansVeg = -cross(va,vb);

		heightC += smudgeDataC.w * u_vegBump;
		heightR += smudgeDataR.w * u_vegBump;
		heightL += smudgeDataL.w * u_vegBump;
		heightU += smudgeDataU.w * u_vegBump;
		heightD += smudgeDataD.w * u_vegBump;

		va = normalize(vec3(u_cellSize.x, heightR-heightL, 0.0f));
		vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y));
		rockNormal = -cross(va,vb);

		out_normalData.zw = rockNormal.xz;
		out_miscData.y = moltenMapResultC;
	}

	////////////////////////////////////////////////////////////////
	// Vegetation
	////////////////////////////////////////////////////////////////
	{
		float vegAmount = out_smudgeData.w;

		vec4 diffuseSampleC = texture(s_diffuseMap, in_uv);

		float slope = rockNormalSansVeg.y;
		float slopeScalar = clamp((slope - u_vegMinSlope) / (u_vegMaxSlope-u_vegMinSlope), 0.0, 1.0);
		slopeScalar = smoothstep( 0.0, 1.0, slopeScalar );

		float dirtHeight = out_heightData.z;
		float dirtScalar = clamp((dirtHeight - u_vegMinDirt) / (u_vegMaxDirt-u_vegMinDirt), 0.0, 1.0);

		float waterHeight = out_heightData.w;
		float waterScalar = 1.0 - min( waterHeight / 0.01, 1.0 );

		float vegAmountTarget = slopeScalar * dirtScalar * waterScalar;
		vegAmountTarget *= diffuseSampleC.y * 2.0;

		vegAmountTarget = smoothstep( 0.0, 0.7, vegAmountTarget );

		vegAmount += min(abs(vegAmountTarget-vegAmount), u_vegGrowthRate);
		vegAmount = min( vegAmount, vegAmountTarget );

		out_smudgeData.w = vegAmount;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Water normal
	//////////////////////////////////////////////////////////////////////////////////
	{
		float moltenMapResultC = out_miscData.y;
		float moltenMapResultL = miscDataL.y;
		float moltenMapResultR = miscDataR.y;
		float moltenMapResultU = miscDataU.y;
		float moltenMapResultD = miscDataD.y;

		float heightC = heightDataC.x + heightDataC.y + heightDataC.z + heightDataC.w + moltenMapResultC * u_mapHeightOffset;
		float heightL = heightDataL.x + heightDataL.y + heightDataL.z + heightDataL.w + moltenMapResultL * u_mapHeightOffset;
		float heightR = heightDataR.x + heightDataR.y + heightDataR.z + heightDataR.w + moltenMapResultR * u_mapHeightOffset;
		float heightU = heightDataU.x + heightDataU.y + heightDataU.z + heightDataU.w + moltenMapResultU * u_mapHeightOffset;
		float heightD = heightDataD.x + heightDataD.y + heightDataD.z + heightDataD.w + moltenMapResultD * u_mapHeightOffset;
		
		vec3 va = normalize(vec3(u_cellSize.x, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y));

		vec3 waterNormal = -cross(va,vb);
		out_normalData.xy = waterNormal.xz;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Occlusion
	//////////////////////////////////////////////////////////////////////////////////
	{
		float occlusion = 0.0f;
		float heightC = heightDataC.x + heightDataC.y + heightDataC.z + miscDataC.y * u_mapHeightOffset + smudgeDataC.w * u_vegBump;

		for ( int i = 1; i < u_numHeightMips; i++ )
		{
			vec4 mippedHeightDataC = textureLod(s_heightData, in_uv, float(i));
			vec4 mippedMiscDataC = textureLod(s_miscData, in_uv, float(i));
			vec4 mippedSmudgeDataC = textureLod(s_smudgeData, in_uv, float(i));

			float mippedHeight = mippedHeightDataC.x + mippedHeightDataC.y + mippedHeightDataC.z + mippedMiscDataC.y * u_mapHeightOffset + mippedSmudgeDataC.w * u_vegBump;
			float diff = max(0.0f, mippedHeight - heightC);
			float ratio = diff / u_cellSize.x;
			float angle = atan(ratio);
			float occlusionFoThisMip = angle / HALF_PI;

			occlusion += occlusionFoThisMip;
		}
		occlusion /= u_numHeightMips;

		out_miscData.w = occlusion;
	}
}