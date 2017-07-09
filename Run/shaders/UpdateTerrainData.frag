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
uniform sampler2D s_uvOffsetData;
uniform sampler2D s_grungeMap;
uniform sampler2D s_fluidVelocityData;

uniform sampler2D s_albedoFluidGradient;

// Mouse
uniform float u_mouseRadius;
uniform float u_mouseMoltenVolumeStrength;
uniform float u_mouseWaterVolumeStrength;
uniform float u_mouseMoltenHeatStrength;
uniform float u_mouseDirtVolumeStrength;
uniform float u_grungeUVRepeat;

// Environment
uniform float u_ambientTemp;
uniform float u_time;

// Global
uniform float u_heightOffset;
uniform bool u_phaseALatch;
uniform bool u_phaseBLatch;

// Molten
uniform float u_heatAdvectSpeed;
uniform float u_moltenViscosity;
uniform float u_rockMeltingPoint;
uniform float u_tempChangeSpeed;
uniform float u_meltCondenseSpeed;
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
vec2 VelocityFromFlux( vec4 fluxC, vec4 fluxL, vec4 fluxR, vec4 fluxU, vec4 fluxD, float viscosity )
{
	return vec2((fluxL.y + fluxC.y) - (fluxR.x + fluxC.x), 
				(fluxU.w + fluxC.w) - (fluxD.z + fluxC.z) ) * viscosity;
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
float CalcMoltenViscosity( float _heat, float _height )
{
	return smoothstep( 0.05, 0.5, _heat ) * u_moltenViscosity;
}

////////////////////////////////////////////////////////////////
//
float exchange( vec4 _heightDataC, vec4 _heightDataN, float _heatC, float _heatN )
{
	float avgHeat = max(_heatC, _heatN);//(_heatC + _heatN) * 0.5;
	float avgHeight = (_heightDataC.y + _heightDataN.y) * 0.5;
	float viscosity = CalcMoltenViscosity( avgHeat, avgHeight );
	
	float diff = max( (_heightDataC.x + _heightDataC.y) - (_heightDataN.x + _heightDataN.y), 0.0 );
	
	float available = _heightDataC.y * 0.125;
	return min( available, diff * viscosity );
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
	//vec4 miscDataTL = texelFetchTL(s_miscData);
	//vec4 miscDataTR = texelFetchTR(s_miscData);
	//vec4 miscDataBL = texelFetchBL(s_miscData);
	//vec4 miscDataBR = texelFetchBR(s_miscData);

	vec4 normalDataC = texelFetchC(s_normalData);
	vec4 velocityDataC = texelFetchC(s_velocityData);
	vec4 smudgeDataC = texelFetchC(s_smudgeData);
	vec4 fluidVelocityC = texelFetchC(s_fluidVelocityData);
	
	vec4 albedoFluidColor = unpackUnorm4x8f( smudgeDataC.z );
	
	vec2 mousePos = GetMousePos();
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-mousePos) / u_mouseRadius);

	
	heightDataC.w = 0.0;
	
	// Assign something sensible to ouputs. We'll be updating these ahead.
	out_heightData = heightDataC;
	out_velocityData = velocityDataC;
	out_miscData = miscDataC;
	out_normalData = normalDataC;
	out_smudgeData = smudgeDataC;

	
	////////////////////////////////////////////////////////////////
	// Update molten
	////////////////////////////////////////////////////////////////
	{
		float prevHeat = miscDataC.x;
		float heatC = miscDataC.x;
		float heightC =  heightDataC.y;
		
		/*
		// Transmit some of this cell's volume to neighbours
		float toL = exchange( heightDataC, heightDataL, miscDataC.x, miscDataL.x );
		float toR = exchange( heightDataC, heightDataR, miscDataC.x, miscDataR.x );
		float toU = exchange( heightDataC, heightDataU, miscDataC.x, miscDataU.x );
		float toD = exchange( heightDataC, heightDataD, miscDataC.x, miscDataD.x );
		
		float totalOut = toL + toR + toU + toD;

		// Bring some volume from neighbours
		float fromL = exchange( heightDataL, heightDataC, miscDataL.x, miscDataC.x );
		float fromR = exchange( heightDataR, heightDataC, miscDataR.x, miscDataC.x );
		float fromU = exchange( heightDataU, heightDataC, miscDataU.x, miscDataC.x );
		float fromD = exchange( heightDataD, heightDataC, miscDataD.x, miscDataC.x );
		
		float totalIn = fromL + fromR + fromU + fromD;

		heightC -= totalOut;
		heightC += totalIn;

		// Heat transfer
		vec4 exportedVolumeAsRatio = vec4( toL, toR, toU, toD ) / max( vec4(heightDataC.y), vec4( 0.00000001 ) );
		float heatExported = dot( vec4(heatC) * exportedVolumeAsRatio, vec4(1.0) );
		
		vec4 importedVolumeAsRatio = 
			vec4( fromL, fromR, fromU, fromD ) / 
			max( vec4( heightDataL.y, heightDataR.y, heightDataU.y, heightDataD.y ), vec4( 0.00000001 ) );
		
		vec4 heatN = vec4( miscDataL.x, miscDataR.x, miscDataU.x, miscDataD.x );
		float heatImported = dot( heatN * importedVolumeAsRatio, vec4(1.0) );
		
		heatC -= heatExported * u_heatAdvectSpeed;
		heatC += heatImported * u_heatAdvectSpeed;
		
		float totalHeatBefore = (miscDataL.x + miscDataR.x + miscDataU.x + miscDataD.x) * 0.25 + miscDataC.x;
		//heatC = min(heatC, totalHeatBefore);
		
		vec2 moltenVelocity = vec2(0.0);
		
		moltenVelocity.x += (importedVolumeAsRatio.x - exportedVolumeAsRatio.y);
		moltenVelocity.x += (exportedVolumeAsRatio.y - importedVolumeAsRatio.y);
		moltenVelocity.y += (importedVolumeAsRatio.z - exportedVolumeAsRatio.z);
		moltenVelocity.y += (exportedVolumeAsRatio.w - importedVolumeAsRatio.w);
		
		float moltenVelocityLength = length(moltenVelocity);
		float moltenVelocityLengthRatio = moltenVelocityLength / 0.05;
		float scalar = 1.0 / (1.0 + moltenVelocityLengthRatio);
		moltenVelocity *= scalar;
		moltenVelocity *= u_moltenVelocityScalar;
		*/
		
		// Cooling
		heatC += (u_ambientTemp - heatC) * u_tempChangeSpeed;

		// Add some lava near the mouse
		/*
		vec4 grungeSample = texture(s_grungeMap, in_uv * u_grungeUVRepeat);
		float heatTextureScalar = mix( 0.3, 0.7, grungeSample.a );
		float volumeTextureScalar = 1.0 - heatTextureScalar;
		float mouseScalar = pow(mouseRatio, 2.0);
		float volumeStrength = mix( 0.5, volumeTextureScalar, min( u_mouseMoltenHeatStrength * 20.0, 1.0 ) );
		heatC   += mouseScalar * u_mouseMoltenHeatStrength * heatTextureScalar * 0.1;
		heightC += mouseScalar * volumeStrength * u_mouseMoltenVolumeStrength * 0.5;
		heatC = max(0.0, heatC);
		*/
		
		//out_heightData.y = heightC;
		out_miscData.x = heatC;
		
		
		//////////////////////////////////////////////////////////////////////////////////
		// Advect albedo fluid
		//////////////////////////////////////////////////////////////////////////////////
		/*
		{
			ivec2 size = textureSize(s_smudgeData, 0);
			vec2 samplePos = in_uv - moltenVelocity.xy * 20.0;
			vec4 albedoFluidSampleGather = textureGather( s_smudgeData, samplePos, 2 );
			
			vec4 newAlbedoFluidColor0 = unpackUnorm4x8f(albedoFluidSampleGather.x);
			vec4 newAlbedoFluidColor1 = unpackUnorm4x8f(albedoFluidSampleGather.y);
			vec4 newAlbedoFluidColor2 = unpackUnorm4x8f(albedoFluidSampleGather.w);
			vec4 newAlbedoFluidColor3 = unpackUnorm4x8f(albedoFluidSampleGather.z);
			
			// bi-linear mixing:
			vec2 filterWeight = fract( (in_uv*size) - vec2(0.5) );
			vec4 temp0 = mix( newAlbedoFluidColor0, newAlbedoFluidColor1, filterWeight.x );
			vec4 temp1 = mix( newAlbedoFluidColor2, newAlbedoFluidColor3, filterWeight.x );
			
			vec4 newAlbedoFluidColor = mix( temp1, temp0, filterWeight.y );
			albedoFluidColor = mix( albedoFluidColor, newAlbedoFluidColor, 0.05 );
		}
		*/
		
		//////////////////////////////////////////////////////////////////////////////////
		// Add variance to albedo color when adding material
		//////////////////////////////////////////////////////////////////////////////////
		/*
		{
			vec4 newAlbedoFluidColor = texture( s_albedoFluidGradient, vec2( u_time * 0.2 + grungeSample.y * 0.3, 0.5 ) );
			float switcher = mouseScalar * u_mouseMoltenVolumeStrength;
			albedoFluidColor = mix(albedoFluidColor, newAlbedoFluidColor, min( switcher * 50.0, 1.0));
		}
		
		out_smudgeData.z = packUnorm4x8f(albedoFluidColor);
		*/
		
		//////////////////////////////////////////////////////////////////////////////////
		// UV OFFSETS
		//////////////////////////////////////////////////////////////////////////////////
		{
			vec4 uvOffsetSample = texelFetchC(s_uvOffsetData);
			vec2 uvOffsetA = uvOffsetSample.xy;
			vec2 uvOffsetB = uvOffsetSample.zw;
			if ( u_phaseALatch )
			{
				uvOffsetA *= 0.0;
			}
			if ( u_phaseBLatch )
			{
				uvOffsetB *= 0.0;
			}
			
			vec2 moltenVelocity = fluidVelocityC.xy * u_moltenVelocityScalar;
			uvOffsetA += moltenVelocity;
			uvOffsetB += moltenVelocity;

			out_uvOffsetData = vec4( uvOffsetA, uvOffsetB );
		}

		//////////////////////////////////////////////////////////////////////////////////
		// SMUDGE MAP
		//////////////////////////////////////////////////////////////////////////////////
		{
			vec2 moltenVelocity = fluidVelocityC.xy;
			vec2 smudgeDir = smudgeDataC.xy;

			float dp = dot( normalize(moltenVelocity), normalize(smudgeDir) );
			if ( isnan(dp) )
				dp = -1.0;

			float ratio = (dp + 1.0) * 0.5;
			smudgeDir += moltenVelocity * u_smudgeChangeRate;

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
	/*
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
	*/
	
	/*
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
	
	////////////////////////////////////////////////////////////////
	// Spawn Foam
	////////////////////////////////////////////////////////////////
	/*
	{
		vec2 waterVelocity = out_velocityData.zw;

		float foamAmount = smudgeDataC.w;
		foamAmount *= u_foamDecayRate;
		foamAmount += length( waterVelocity ) * u_foamSpawnStrength;
		foamAmount = min(1.0, foamAmount);

		out_smudgeData.w = foamAmount;
	}
	*/
	
	////////////////////////////////////////////////////////////////
	// Melt/condense rock
	////////////////////////////////////////////////////////////////
	{
		float heat = out_miscData.x;

		float rockHeight = out_heightData.x;
		float moltenHeight = out_heightData.y;
		float combinedHeight = moltenHeight + rockHeight;

		float targetRatio = clamp( heat, 0.0, 1.0 );
		float targetMoltenHeight = targetRatio * combinedHeight;

		float meltCondenseSpeed = (1.0-clamp(heat, 0.0, 1.0)) * u_meltCondenseSpeed;

		float newMolten = moltenHeight + (targetMoltenHeight - moltenHeight) * meltCondenseSpeed;
		float newRock = combinedHeight - newMolten;

		out_heightData.x = newRock;
		out_heightData.y = newMolten;


		float dirtHeight = out_heightData.z;
		float dirtToMolten = min( dirtHeight, heat * u_meltCondenseSpeed * 0.1 );
		//out_heightData.x += dirtToMolten * u_dirtDensity;
		//out_heightData.z -= dirtToMolten;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Solid normal
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
		
		vec3 va = normalize(vec3(u_cellSize.x*2.0, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y*2.0));
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
			strength *= 1.25;
		}
		occlusion /= totalStrength;
		occlusion = min(1.0, occlusion);

		out_miscData.w = occlusion;

		out_heightData = max( vec4(0.0), out_heightData );
	}
}