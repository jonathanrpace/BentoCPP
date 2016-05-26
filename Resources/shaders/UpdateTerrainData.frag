#version 430 core

const vec4 EPSILON = vec4(0.001f);
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

// Water
uniform float u_waterViscosity;
uniform float u_waterVelocityScalar;
uniform float u_waterVelocityDamping;
uniform float u_evapourationRate;
uniform float u_rainRate;

// Erosion
uniform float u_erosionStrength;
uniform float u_erosionMaxDepth;
uniform float u_erosionWaterDepthMin;
uniform float u_erosionWaterDepthMax;
uniform float u_erosionWaterSpeedMax;

// Dirt transport
uniform float u_dirtTransportSpeed;
uniform float u_dirtPickupSpeed;
uniform float u_dirtDepositSpeed;
uniform float u_dirtErodeSpeedMax;

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
// Main
////////////////////////////////////////////////////////////////
void main(void)
{ 
	ivec2 texelCoordC = ivec2(gl_FragCoord.xy);
	ivec2 texelCoordL = texelCoordC - ivec2(1,0);
	ivec2 texelCoordR = texelCoordC + ivec2(1,0);
	ivec2 texelCoordU = texelCoordC - ivec2(0,1);
	ivec2 texelCoordD = texelCoordC + ivec2(0,1);

	// Shared data samples
	vec4 heightDataC = texelFetch(s_heightData, texelCoordC, 0);
	vec4 heightDataL = texelFetch(s_heightData, texelCoordL, 0);
	vec4 heightDataR = texelFetch(s_heightData, texelCoordR, 0);
	vec4 heightDataU = texelFetch(s_heightData, texelCoordU, 0);
	vec4 heightDataD = texelFetch(s_heightData, texelCoordD, 0);

	vec4 miscDataC = texelFetch(s_miscData, texelCoordC, 0);
	vec4 miscDataL = texelFetch(s_miscData, texelCoordL, 0);
	vec4 miscDataR = texelFetch(s_miscData, texelCoordR, 0);
	vec4 miscDataU = texelFetch(s_miscData, texelCoordU, 0);
	vec4 miscDataD = texelFetch(s_miscData, texelCoordD, 0);

	vec4 velocityDataC = texelFetch(s_velocityData, texelCoordC, 0);
	vec4 smudgeDataC = texelFetch(s_smudgeData, texelCoordC, 0);

	vec4 diffuseSampleC = texture(s_diffuseMap, in_uv);
	
	vec2 mousePos = GetMousePos();
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-mousePos) / u_mouseRadius);

	// Assign something sensible to ouputs
	// We'll be updating these ahead
	out_heightData = heightDataC;
	out_velocityData = velocityDataC;
	out_miscData = miscDataC;
	out_normalData = vec4(0.0);
	out_smudgeData = smudgeDataC;

	////////////////////////////////////////////////////////////////
	// Update molten
	////////////////////////////////////////////////////////////////
	{
		float heat = miscDataC.x;
		float height = heightDataC.y;

		float viscosity = CalcMoltenViscosity(heat);

		vec4 fluxC = texelFetch(s_rockFluxData, texelCoordC, 0);
		vec4 fluxL = texelFetch(s_rockFluxData, texelCoordL, 0);
		vec4 fluxR = texelFetch(s_rockFluxData, texelCoordR, 0);
		vec4 fluxU = texelFetch(s_rockFluxData, texelCoordU, 0);
		vec4 fluxD = texelFetch(s_rockFluxData, texelCoordD, 0);
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
		heat += (u_ambientTemp - heat) * u_tempChangeSpeed * (1.0-occlusion);

		// Add some lava near the mouse
		float mouseTextureScalar = diffuseSampleC.x;
		float mouseTextureScalar2 = 1.0-diffuseSampleC.x;
		heat   += ( pow(mouseRatio, 0.5) * u_mouseMoltenHeatStrength   * mix(1.00, 1.0, mouseTextureScalar) ) / (1.0001+heat*5.0);
		height += ( pow(mouseRatio, 1.5) * u_mouseMoltenVolumeStrength * mix(0.05, 1.0, mouseTextureScalar2) ) / (1.0001+height);

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

			// How much influence does the current velocity have on the existing smudge?
			// Velocities pointing the same way have 0% influence, opposite = 100%

			float influence = 1.0;
			if ( length(velocity) > EPSILON.x )
			{
				if ( length(smudgeUV) > EPSILON.x )
				{
					influence = 1.0 - ((dot(normalize(smudgeUV), normalize(velocity)) + 1.0) * 0.5);
				}
				
				influence *= length(velocity);
			}

			influence *= (1.0 - length(smudgeUV));

			smudgeUV += velocity * influence;


			out_smudgeData.xy = smudgeUV;
			
			/*
			////////////////////////////////////////////////////////////////
			// SEPERATION
			// Visit each 9 neighbours, and determine which ones will sample from within this.
			// cell. Those that do exert a 'pull' on this cell from their direction. If this 
			// pull is counteracted by another neighbour in the opposite direction then this 
			// cell is being 'seperated' and should cycle its sampled colour so that 
			// neighbours pulling this colour get fresh detail, rather than smearing a single 
			// value
			////////////////////////////////////////////////////////////////
			float seperationAmount = 0.0f;
			ivec2 coord = texelCoordC / 2;
			float strength = 1.0f;
			for ( int i = 1; i < 8; i++ )
			{
				vec4 fluxN = textureLod( s_rockFluxData, in_uv, i );
				vec2 velocityN = vec2( fluxN.y-fluxN.x, fluxN.w-fluxN.z ) * viscosity;

				float seperationDot = 1.0f-clamp( dot( normalize(velocityN), normalize(velocity) ), 0.0f, 1.0f );
				float seperationScalar = length(velocityN) + length(velocity);
				seperationAmount += seperationScalar * seperationDot * strength;
			
				coord /= 2;
				strength *= 0.65f;
			}
			
			vec2 pullUV = in_uv - velocity * u_textureScrollSpeed;
			float newPhase = texture(s_moltenMapData, pullUV).x;
			newPhase += min(u_cycleSpeed * seperationAmount, 0.1f);
			newPhase = mod(newPhase, 1.0f);
			out_miscData.y = newPhase;

			////////////////////////////////////////////////////////////////
			*/
		}
	}

	////////////////////////////////////////////////////////////////
	// Update water
	////////////////////////////////////////////////////////////////
	{
		float waterHeight = heightDataC.w;

		vec4 fluxC = texelFetch(s_waterFluxData, texelCoordC, 0);
		vec4 fluxL = texelFetch(s_waterFluxData, texelCoordL, 0);
		vec4 fluxR = texelFetch(s_waterFluxData, texelCoordR, 0);
		vec4 fluxU = texelFetch(s_waterFluxData, texelCoordU, 0);
		vec4 fluxD = texelFetch(s_waterFluxData, texelCoordD, 0);
		vec4 fluxN = vec4(fluxL.y, fluxR.x, fluxU.w, fluxD.z);

		// Update water height based on flux
		float fluxChange = ((fluxN.x+fluxN.y+fluxN.z+fluxN.w)-(fluxC.x+fluxC.y+fluxC.z+fluxC.w));
		waterHeight += fluxChange * u_waterViscosity;

		// Add some water near the mouse
		waterHeight += pow(mouseRatio, 1.1) * u_mouseWaterVolumeStrength;

		waterHeight += u_rainRate;
		waterHeight -= u_evapourationRate;
		waterHeight = max(waterHeight, 0.0);

		out_heightData.w = waterHeight;

		////////////////////////////////////////////////////////////////
		// Wave noise
		////////////////////////////////////////////////////////////////
		/*
		waveNoiseHeight = waveNoise(in_uv);
		waveNoiseHeight *= u_waveAmplitude;
		*/
		
		////////////////////////////////////////////////////////////////
		// Erosion
		////////////////////////////////////////////////////////////////
		// Only erode up to a certain depth
		float dirtHeight = heightDataC.z;
		float erosionDirtDepthScalar = 1.0 - min(dirtHeight / u_erosionMaxDepth, 1.0);

		// Erode more as water depth increases from zero, up to a point, then diminish.
		float erosionWaterDepthScalar =        smoothstep(0.0,                    u_erosionWaterDepthMin, waterHeight) 
										* (1.0-smoothstep(u_erosionWaterDepthMin, u_erosionWaterDepthMax, waterHeight));

		// Erode more if water is moving fast
		vec2 waterVelocity = velocityDataC.zw;
		float waterSpeedC = length(waterVelocity);
		if ( isnan(waterSpeedC) ) waterSpeedC = 0.0;
		if ( isinf(waterSpeedC) ) waterSpeedC = 0.0;
		float erosionWaterSpeedScalar = smoothstep( 0.0, u_erosionWaterSpeedMax, waterSpeedC );

		float solidHeight = heightDataC.x;
		float rockToDirt = min( erosionDirtDepthScalar * erosionWaterDepthScalar * erosionWaterSpeedScalar * u_erosionStrength, solidHeight );
		solidHeight -= rockToDirt;
		dirtHeight += rockToDirt;
		
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
		float pickUpRate = pow( min( waterSpeedC / u_dirtErodeSpeedMax, 1.0 ), 1.2 );
		float amountPickedUp = min( pickUpRate * u_dirtPickupSpeed, dirtHeight );

		dirtHeight -= amountPickedUp;
		dissolvedDirt += amountPickedUp;
		dissolvedDirt = max(0.0,dissolvedDirt);

		////////////////////////////////////////////////////////////////
		// Move dissolved dirt between cells
		////////////////////////////////////////////////////////////////
		float transferedAwayX = min( abs(waterVelocity.x) * u_dirtTransportSpeed, dissolvedDirt * 0.25 );
		float transferedAwayY = min( abs(waterVelocity.y) * u_dirtTransportSpeed, dissolvedDirt * 0.25 );
		float transferedAway = transferedAwayX + transferedAwayY;

		float dissolvedDirtL = miscDataL.w;
		float dissolvedDirtR = miscDataR.w;
		float dissolvedDirtU = miscDataU.w;
		float dissolvedDirtD = miscDataD.w;

		vec2 waterVelocityL = texelFetch(s_velocityData, texelCoordL, 0).xy;
		vec2 waterVelocityR = texelFetch(s_velocityData, texelCoordR, 0).xy;
		vec2 waterVelocityU = texelFetch(s_velocityData, texelCoordU, 0).xy;
		vec2 waterVelocityD = texelFetch(s_velocityData, texelCoordD, 0).xy;

		float transferedInL = min( max( waterVelocityL.x,0.0) * u_dirtTransportSpeed, dissolvedDirtL * 0.25 );
		float transferedInR = min( max(-waterVelocityR.x,0.0) * u_dirtTransportSpeed, dissolvedDirtR * 0.25 );
		float transferedInU = min( max( waterVelocityU.y,0.0) * u_dirtTransportSpeed, dissolvedDirtU * 0.25 );
		float transferedInD = min( max(-waterVelocityD.y,0.0) * u_dirtTransportSpeed, dissolvedDirtD * 0.25 );

		float transferedIn = transferedInL + transferedInR + transferedInU + transferedInD;

		dissolvedDirt += (transferedIn - transferedAway) * 0.1;
		dissolvedDirt = max(0.0, dissolvedDirt);

		out_miscData.z = dissolvedDirt;

		////////////////////////////////////////////////////////////////
		// Update water velocity
		////////////////////////////////////////////////////////////////
		waterVelocity += VelocityFromFlux( fluxC, fluxL, fluxR, fluxU, fluxD, u_waterViscosity ) * u_waterVelocityScalar;
		waterVelocity *= u_waterVelocityDamping;

		out_velocityData.zw = waterVelocity;
	}

	////////////////////////////////////////////////////////////////
	// Melt/condense rock
	////////////////////////////////////////////////////////////////
	{
		float maxTemp = u_rockMeltingPoint * 5.0;
		float heat = out_miscData.x;
		float moltenHeight = out_heightData.y;

		float solidToMolten = smoothstep(u_rockMeltingPoint, maxTemp, heat) * u_meltSpeed;
		solidToMolten = min(solidToMolten, heightDataC.x);

		float moltenToSolid = (1.0-smoothstep(0, u_rockMeltingPoint, miscDataC.x)) * u_condenseSpeed;
		moltenToSolid = min(moltenToSolid, moltenHeight);

		out_heightData.x -= solidToMolten;
		out_heightData.x += moltenToSolid;

		out_heightData.y -= moltenToSolid;
		out_heightData.y += solidToMolten;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Rock normal
	//////////////////////////////////////////////////////////////////////////////////
	{
		float moltenMapC = texelFetch(s_moltenMapData, texelCoordC, 0).x;
		float moltenMapL = texelFetch(s_moltenMapData, texelCoordL, 0).x;
		float moltenMapR = texelFetch(s_moltenMapData, texelCoordR, 0).x;
		float moltenMapU = texelFetch(s_moltenMapData, texelCoordU, 0).x;
		float moltenMapD = texelFetch(s_moltenMapData, texelCoordD, 0).x;

		float moltenMapScalar = 0.001;
		moltenMapC *= moltenMapScalar;
		moltenMapL *= moltenMapScalar;
		moltenMapR *= moltenMapScalar;
		moltenMapU *= moltenMapScalar;
		moltenMapD *= moltenMapScalar;

		float heightR = heightDataR.x + heightDataR.y + heightDataR.z + moltenMapR;
		float heightL = heightDataL.x + heightDataL.y + heightDataL.z + moltenMapL;
		float heightU = heightDataU.x + heightDataU.y + heightDataU.z + moltenMapU;
		float heightD = heightDataD.x + heightDataD.y + heightDataD.z + moltenMapD;
		float heightC = heightDataC.x + heightDataC.y + heightDataC.z + moltenMapC;

		vec3 va = normalize(vec3(u_cellSize.x, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y));
		vec3 rockNormal = -cross(va,vb);

		out_normalData.zw = rockNormal.xz;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Water normal
	//////////////////////////////////////////////////////////////////////////////////
	{
		float heightR = heightDataR.x + heightDataR.y + heightDataR.z + heightDataR.w;
		float heightL = heightDataL.x + heightDataL.y + heightDataL.z + heightDataL.w;
		float heightU = heightDataU.x + heightDataU.y + heightDataU.z + heightDataU.w;
		float heightD = heightDataD.x + heightDataD.y + heightDataD.z + heightDataD.w;
		float heightC = heightDataC.x + heightDataC.y + heightDataC.z + heightDataC.w;
		
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
		float heightC = heightDataC.x + heightDataC.y + heightDataC.z;

		for ( int i = 1; i < u_numHeightMips; i++ )
		{
			vec4 mippedHeightDataC = textureLod(s_heightData, in_uv, float(i));
			float mippedHeight = mippedHeightDataC.x + mippedHeightDataC.y + mippedHeightDataC.z;
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