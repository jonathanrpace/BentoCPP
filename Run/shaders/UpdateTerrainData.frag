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
uniform float u_moltenSlopeStrength;
uniform float u_moltenVelocityDamping;

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
layout( location = 6 ) out vec4 out_fluidVelocityData;

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
	ivec2 T = ivec2(gl_FragCoord.xy);
	
	vec4 hC = texelFetchC(s_heightData);
	vec4 hN = texelFetchU(s_heightData);
	vec4 hS = texelFetchD(s_heightData);
	vec4 hE = texelFetchR(s_heightData);
	vec4 hW = texelFetchL(s_heightData);
	
	vec4 fC = texelFetchC(s_fluidVelocityData);
	vec4 fN = texelFetchU(s_fluidVelocityData);
	vec4 fS = texelFetchD(s_fluidVelocityData);
	vec4 fE = texelFetchR(s_fluidVelocityData);
	vec4 fW = texelFetchL(s_fluidVelocityData);

	vec4 mC = texelFetchC(s_miscData);
	vec4 mL = texelFetchL(s_miscData);
	vec4 mR = texelFetchR(s_miscData);
	vec4 mU = texelFetchU(s_miscData);
	vec4 mD = texelFetchD(s_miscData);

	vec4 normalDataC = texelFetchC(s_normalData);
	vec4 uvOffsetDataC = texelFetchC(s_uvOffsetData);
	vec4 velocityDataC = texelFetchC(s_velocityData);
	vec4 smudgeDataC = texelFetchC(s_smudgeData);
	
	
	// Apply input
	{
		vec2 mousePos = GetMousePos();
		float mouseRatio = 1.0f - min(1.0f, length(in_uv-mousePos) / u_mouseRadius);
		float mouseScalar = pow(mouseRatio, 2.0);
		float volumeStrength = mouseScalar * u_mouseMoltenVolumeStrength;
		mC.x += mouseScalar * u_mouseMoltenHeatStrength * 0.1;
		hC.y += mouseScalar * u_mouseMoltenVolumeStrength * 0.5;
		
		hC.x += mouseScalar * u_mouseDirtVolumeStrength * 0.5;
		
		/*
		{
			float dirtHeight = out_heightData.z;

			dirtHeight += pow(mouseRatio, 0.7) * u_mouseDirtVolumeStrength;
			out_heightData.z = dirtHeight;
		}
		*/
	}
	
	// Cooling
	mC.x += (u_ambientTemp - mC.x) * u_tempChangeSpeed;
	
	// Advect molten volume and heat
	{
		float toN = min( fC.z * hC.y, hC.y * 0.15 );
		float toS = min( fC.w * hC.y, hC.y * 0.15 );
		float toE = min( fC.y * hC.y, hC.y * 0.15 );
		float toW = min( fC.x * hC.y, hC.y * 0.15 );
		float totalTo = (toN + toS + toE + toW);

		float fromN = min( fN.w * hN.y, hN.y * 0.15 );
		float fromS = min( fS.z * hS.y, hS.y * 0.15 );
		float fromE = min( fE.x * hE.y, hE.y * 0.15 );
		float fromW = min( fW.y * hW.y, hW.y * 0.15 );

		float totalFrom = fromN + fromS + fromE + fromW;

		hC.y += totalFrom;
		hC.y -= totalTo;
		
		// Advect heat
		float heatN = texelFetchOffset(s_miscData, T, 0, ivec2( 0, -1)).x;
		float heatS = texelFetchOffset(s_miscData, T, 0, ivec2( 0,  1)).x;
		float heatE = texelFetchOffset(s_miscData, T, 0, ivec2( 1,  0)).x;
		float heatW = texelFetchOffset(s_miscData, T, 0, ivec2(-1,  0)).x;
		
		float toProp = totalTo / max( hC.y, EPSILON.x );
		float toHeat = toProp * mC.x;
		
		vec4 fromProp = vec4(fromN, fromS, fromE, fromW) / max( vec4( hN.y, hS.y, hE.y, hW.y ), EPSILON );
		float fromheat = dot( vec4( heatN, heatS, heatE, heatW ) * fromProp, vec4(1.0) );
		
		mC.x -= toHeat;
		mC.x += fromheat;
	}
	
	// Add slope to molten flux
	{
		// Dampen
		fC *= u_moltenVelocityDamping;
		
		float mhC = hC.x + hC.y;
		float mhN = hN.x + hN.y;
		float mhS = hS.x + hS.y;
		float mhE = hE.x + hE.y;
		float mhW = hW.x + hW.y;
		
		fC.x += (mhC - mhW) * u_moltenSlopeStrength;
		fC.y += (mhC - mhE) * u_moltenSlopeStrength;
		fC.z += (mhC - mhN) * u_moltenSlopeStrength;
		fC.w += (mhC - mhS) * u_moltenSlopeStrength;
		
		fC *= smoothstep( 0.0, 0.001, hC.y );
		
		// Limit
		fC = max( fC, vec4(0.0));
	}
	
	{
		vec2 velocity = vec2(fC.y-fC.x, fC.w-fC.z);
		
		//////////////////////////////////////////////////////////////////////////////////
		// UV OFFSETS
		//////////////////////////////////////////////////////////////////////////////////
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
			
			uvOffsetA += velocity * u_moltenVelocityScalar;
			uvOffsetB += velocity * u_moltenVelocityScalar;
			
			uvOffsetDataC = vec4( uvOffsetA, uvOffsetB );
		}

		//////////////////////////////////////////////////////////////////////////////////
		// SMUDGE MAP
		//////////////////////////////////////////////////////////////////////////////////
		{
			float dp = dot( normalize(velocity), normalize(smudgeDataC.xy) );
			if ( isnan(dp) )
				dp = -1.0;

			float ratio = (dp + 1.0) * 0.5;
			smudgeDataC.xy += velocity * u_smudgeChangeRate;
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
	/*
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
	*/
	
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

		float heightC = hC.x + hC.y + hC.z;
		float heightR = hE.x + hE.y + hE.z;
		float heightL = hW.x + hW.y + hW.z;
		float heightU = hN.x + hN.y + hN.z;
		float heightD = hS.x + hS.y + hS.z;
		
		vec3 va = normalize(vec3(u_cellSize.x*2.0, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y*2.0));
		vec3 rockNormal = -cross(va,vb);

		normalDataC.zw = rockNormal.xz;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Water normal
	//////////////////////////////////////////////////////////////////////////////////
	/*
	{
		float heightC = heightDataC.x + heightDataC.y + heightDataC.z + heightDataC.w;
		float heightL = heightDataL.x + heightDataL.y + heightDataL.z + heightDataL.w;
		float heightR = heightDataR.x + heightDataR.y + heightDataR.z + heightDataR.w;
		float heightU = heightDataU.x + heightDataU.y + heightDataU.z + heightDataU.w;
		float heightD = heightDataD.x + heightDataD.y + heightDataD.z + heightDataD.w;

		vec3 va = normalize(vec3(u_cellSize.x*2.0, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y*2.0));

		vec3 waterNormal = -cross(va,vb);
		normalDataC.xy = waterNormal.xz;
	}
	*/
	
	//////////////////////////////////////////////////////////////////////////////////
	// Occlusion
	//////////////////////////////////////////////////////////////////////////////////
	{
		float occlusion = 0.0f;
		float heightC = hC.x + hC.y + hC.z;

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

		mC.w = occlusion;
	}
	
	if ( hC.w > 0.5 )
		hC.w = 0.0;
	
	out_heightData = max( vec4(0.0), hC );
	out_velocityData = velocityDataC;
	out_miscData = mC;
	out_normalData = normalDataC;
	out_smudgeData = smudgeDataC;
	out_fluidVelocityData = fC;
	out_uvOffsetData = uvOffsetDataC;
}