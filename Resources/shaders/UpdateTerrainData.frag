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
	return diff * _scalar * viscosity;
}

float exchangeHeat( float _volumeFromN, vec4 _heightDataN, vec4 _miscDataN, float _limit )
{
	float advectSpeed = mix( u_heatAdvectSpeed * 100.0, u_heatAdvectSpeed, clamp( _volumeFromN / 0.1, 0.0, 1.0 ) );
	return min( _volumeFromN / max(_heightDataN.y, 0.01), _limit) * _miscDataN.x * u_heatAdvectSpeed;
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

		// Add some lava near the mouse
		vec4 diffuseSampleC = texture(s_diffuseMap, in_uv+mousePos*0.1);
		float heatTextureScalar = pow( 1.0-diffuseSampleC.x, 2.0 );
		float heightTextureScalar = pow( diffuseSampleC.x, 2.0 );
		heatC   += ( pow(mouseRatio, 2.0) * u_mouseMoltenHeatStrength   * mix(0.01, 0.1, heatTextureScalar) ) / (1.0+heatC*10.0);
		heightC += ( pow(mouseRatio, 2.0) * u_mouseMoltenVolumeStrength * mix(0.5, 0.6, heightTextureScalar) ) / (1.0+heightC);
		heatC = max(0.0, heatC);

		//out_smudgeData.z += mouseRatio * u_mouseMoltenHeatStrength * 0.1;

		out_heightData.y = heightC;
		out_miscData.x = heatC;

		//////////////////////////////////////////////////////////////////////////////////
		// MOLTEN MAPPING
		//////////////////////////////////////////////////////////////////////////////////
		{
			vec2 velocity = velocityDataC.xy;
			out_velocityData.xy = moltenVelocity * u_moltenVelocityScalar;

			vec2 smudgeUV = smudgeDataC.xy;

			float smudgeAmount = length(smudgeUV+EPSILON.xy);
			float moltenSpeed = length(velocity+EPSILON.xy);

			float influence = moltenSpeed * 100.0;
			float dp = clamp( dot( normalize(velocity+EPSILON.xy), normalize(smudgeUV+EPSILON.xy) ), 0.0, 1.0 );
			dp = 1.0 - dp;

			vec2 velocityN = min( velocity / moltenSpeed, vec2(1.0) );
			smudgeUV += velocityN * influence * 0.2 * dp;

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

		// Add a bit of smoke for general heat
		out_smudgeData.z += pow(heat, 2.0) * 0.03;
		
		
		////////////////////////////////////////////////////////////////
		// Erosion
		////////////////////////////////////////////////////////////////

		// Only erode up to a certain depth
		float dirtHeight = out_heightData.z;
		float erosionDirtDepthScalar = 1.0 - min(dirtHeight / u_erosionMaxDepth, 1.0);

		// Erode more as water depth increases from zero, up to a point, then diminish.
		float erosionWaterDepthScalar =        smoothstep(0.0,                    u_erosionWaterDepthMin, waterHeight) 
										* (1.0-smoothstep(u_erosionWaterDepthMin, u_erosionWaterDepthMax, waterHeight));

		// Erode more if water is moving fast
		vec2 waterVelocityC = velocityDataC.zw;
		float waterSpeedC = 1.0;//length(waterVelocityC);
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

		float moltenToSolid = (1.0-smoothstep(0.0, 0.5, miscDataC.x)) * u_condenseSpeed;
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

		float strength = 1.0;
		float totalStrength = 0.0;
		float moltenMapResultC = 0.0;
		for ( int i = 0; i < 1; i++ )
		{
			moltenMapResultC += textureLod(s_moltenMapData, uvC, i).x * strength;

			totalStrength += strength;
			strength *= 0.0;
		}
		moltenMapResultC /= totalStrength;

		float moltenMapResultL = miscDataL.y;
		float moltenMapResultR = miscDataR.y;
		float moltenMapResultU = miscDataU.y;
		float moltenMapResultD = miscDataD.y;

		float heightC = heightDataC.x + heightDataC.y + heightDataC.z; + moltenMapResultC * u_mapHeightOffset;
		float heightR = heightDataR.x + heightDataR.y + heightDataR.z; + moltenMapResultR * u_mapHeightOffset;
		float heightL = heightDataL.x + heightDataL.y + heightDataL.z; + moltenMapResultL * u_mapHeightOffset;
		float heightU = heightDataU.x + heightDataU.y + heightDataU.z; + moltenMapResultU * u_mapHeightOffset;
		float heightD = heightDataD.x + heightDataD.y + heightDataD.z; + moltenMapResultD * u_mapHeightOffset;
		
		vec3 va = normalize(vec3(u_cellSize.x, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y));

		va = normalize(vec3(u_cellSize.x, heightR-heightL, 0.0f));
		vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y));
		rockNormal = -cross(va,vb);

		out_normalData.zw = rockNormal.xz;
		out_miscData.y = moltenMapResultC;
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
		float heightC = heightDataC.x + heightDataC.y + heightDataC.z + miscDataC.y * u_mapHeightOffset + smudgeDataC.w;

		float strength = 1.0;
		float totalStrength = 0.0;
		for ( int i = 1; i < u_numHeightMips; i++ )
		{
			vec4 mippedHeightDataC = textureLod(s_heightData, in_uv, float(i));
			vec4 mippedMiscDataC = textureLod(s_miscData, in_uv, float(i));
			vec4 mippedSmudgeDataC = textureLod(s_smudgeData, in_uv, float(i));

			float mippedHeight = mippedHeightDataC.x + mippedHeightDataC.y + mippedHeightDataC.z + mippedMiscDataC.y * u_mapHeightOffset + mippedSmudgeDataC.w;
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

		out_miscData.w = occlusion;
	}
}