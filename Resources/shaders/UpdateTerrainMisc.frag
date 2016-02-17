#version 430 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Samplers
uniform sampler2D s_rockData;
uniform sampler2D s_rockFluxData;
uniform sampler2D s_waterData;
uniform sampler2D s_waterFluxData;
uniform sampler2D s_mappingData;
uniform sampler2D s_diffuseMap;

// From VS
in Varying
{
	vec2 in_uv;
};

uniform float u_textureScrollSpeed;
uniform float u_cycleSpeed;
uniform float u_mapHeightOffset;

uniform float u_viscosityMin;
uniform float u_viscosityMax;
uniform float u_heatViscosityPower;
uniform float u_heatViscosityBias;

uniform float u_waterViscosity;

uniform float u_mouseRadius;
uniform float u_mouseVolumeStrength;
uniform float u_mouseHeatStrength;

uniform vec2 u_cellSize;

uniform int u_numHeightMips;

uniform float u_time = 0.0f;


// Buffers
layout( std430, binding = 0 ) buffer MousePositionBuffer
{
	int mouseBufferZ;
	int mouseBufferU;
	int mouseBufferV;
};

// Outputs
layout( location = 0 ) out vec4 out_mappingData;
layout( location = 1 ) out vec4 out_rockNormal;
layout( location = 2 ) out vec4 out_waterNormal;

////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////

float CalcViscosity( float _heat, float _viscosityScalar )
{
	float viscosity = mix( u_viscosityMin, u_viscosityMax, 1.0f-_viscosityScalar );
	return pow(smoothstep( 0.0f, 1.0f, clamp(_heat-u_heatViscosityBias, 0.0f, 1.0f)), u_heatViscosityPower) * viscosity;
}

vec2 GetMousePos()
{
	vec2 mousePos = vec2( mouseBufferU, mouseBufferV ) / 255;
	return mousePos;
}

vec2 VelocityFromFlux( vec4 fluxC, vec4 fluxL, vec4 fluxR, vec4 fluxU, vec4 fluxD, vec2 texelSize, float viscosity )
{
	vec2 velocity = vec2(	(fluxL.y + fluxC.y) - (fluxR.x + fluxC.x), 
							(fluxU.w + fluxC.w) - (fluxD.z + fluxC.z) );
	velocity /= texelSize;
	velocity *= viscosity;

	return velocity;
}

float hash( vec2 p ) 
{
	float h = dot(p,vec2(127.1,311.7));	
    return fract(sin(h)*43758.5453123);
}

float noise( in vec2 p ) 
{
    vec2 i = floor( p );
    vec2 f = fract( p );	
	vec2 u = f*f*(3.0-2.0*f);
    return -1.0+2.0*mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}

float sea_octave(vec2 uv, float choppy)
{
    uv += noise(uv);        
    vec2 wv = 1.0-abs(sin(uv));
    vec2 swv = abs(cos(uv));    
    wv = mix(wv,swv,wv);
    return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
}

float map_detailed(vec3 p, float choppy)
{
	const float SEA_CHOPPY = 2.0;
	const float SEA_SPEED = 0.1f;
	const float SEA_FREQ = 1.0;

	mat2 octave_m = mat2(1.6,1.2,-1.2,1.6);

    float freq = SEA_FREQ;
    float amp = 1.0f;
	float speed = SEA_SPEED;
    choppy = 1.0f + choppy * SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < 5; i++)
	 {        
    	d = sea_octave((uv+u_time*SEA_SPEED)*freq,choppy);
    	d += sea_octave((uv-u_time*SEA_SPEED)*freq,choppy);
        h += d * amp;        
    	uv *= octave_m; 
		freq *= 2.2; 
		amp *= 0.3;
		speed *= 1.5;
    }
    return p.y - h;
}



////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////

void main(void)
{
	ivec2 dimensions = textureSize( s_rockData, 0 );
	vec2 texelSize = 1.0f / dimensions;

	ivec2 texelCoordC = ivec2(gl_FragCoord.xy);
	ivec2 texelCoordL = texelCoordC - ivec2(1,0);
	ivec2 texelCoordR = texelCoordC + ivec2(1,0);
	ivec2 texelCoordU = texelCoordC - ivec2(0,1);
	ivec2 texelCoordD = texelCoordC + ivec2(0,1);

	vec4 rockDataC = texelFetch(s_rockData, texelCoordC, 0);
	vec4 rockDataL = texelFetch(s_rockData, texelCoordL, 0);
	vec4 rockDataR = texelFetch(s_rockData, texelCoordR, 0);
	vec4 rockDataU = texelFetch(s_rockData, texelCoordU, 0);
	vec4 rockDataD = texelFetch(s_rockData, texelCoordD, 0);

	vec4 waterDataC = texelFetch(s_waterData, texelCoordC, 0);
	vec4 waterDataL = texelFetch(s_waterData, texelCoordL, 0);
	vec4 waterDataR = texelFetch(s_waterData, texelCoordR, 0);
	vec4 waterDataU = texelFetch(s_waterData, texelCoordU, 0);
	vec4 waterDataD = texelFetch(s_waterData, texelCoordD, 0);

	vec4 mappingDataC = texelFetch(s_mappingData, texelCoordC, 0);
	vec4 mappingDataL = texelFetch(s_mappingData, texelCoordL, 0);
	vec4 mappingDataR = texelFetch(s_mappingData, texelCoordR, 0);
	vec4 mappingDataU = texelFetch(s_mappingData, texelCoordU, 0);
	vec4 mappingDataD = texelFetch(s_mappingData, texelCoordD, 0);

	

	float heat = rockDataC.z;
	float viscosity = CalcViscosity(heat, mappingDataC.y);

	//////////////////////////////////////////////////////////////////////////////////
	// Update Mapping
	// Visit each 9 neighbours, and determine which ones will sample from within this.
	// cell. Those that do exert a 'pull' on this cell from their direction. If this 
	// pull is counteracted by another neighbour in the opposite direction then this 
	// cell is being 'seperated' and should cycle its sampled colour so that 
	// neighbours pulling this colour get fresh detail, rather than smearing a single 
	// value
	//////////////////////////////////////////////////////////////////////////////////
	{

		vec4 fluxC  = texelFetch(s_rockFluxData, texelCoordC, 0);
		vec4 fluxL  = texelFetch(s_rockFluxData, texelCoordL, 0);
		vec4 fluxR  = texelFetch(s_rockFluxData, texelCoordR, 0);
		vec4 fluxU  = texelFetch(s_rockFluxData, texelCoordU, 0);
		vec4 fluxD  = texelFetch(s_rockFluxData, texelCoordD, 0);

		vec2 velocity = VelocityFromFlux(fluxC, fluxL, fluxR, fluxU, fluxD, texelSize, viscosity );

		////////////////////////////////////////////////////////////////
		// SEPERATION/COMPRESSION
		////////////////////////////////////////////////////////////////

		//vec2 velocity = vec2( fluxC.y - fluxC.x, fluxC.w - fluxC.z );
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
		////////////////////////////////////////////////////////////////

		vec2 pullUV = in_uv - velocity * u_textureScrollSpeed * texelSize;
		vec4 newMappingData = texture(s_mappingData, pullUV);

		float newPhase = newMappingData.x;

		//mappingDataC.x = clamp( pulledValue.x, 0.0f, 1.0f );
		
		newPhase += min(u_cycleSpeed * seperationAmount, 0.1f);
		newPhase = mod(newPhase, 1.0f);

		
		float samplePos = newPhase;

		float diffuseSampleA = texture(s_diffuseMap, pullUV).y;
		samplePos += diffuseSampleA * 0.05;// * (1.0f - clamp((heat-u_heatViscosityBias)*0.5f, 0.0f, 1.0f)) * 0.05f;
		float newBump = texture(s_diffuseMap, vec2(samplePos,0)).x;
		

		mappingDataC.x = newPhase;
		mappingDataC.y = newBump;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Calculate water normal
	//////////////////////////////////////////////////////////////////////////////////
	vec3 waterNormal;
	{
		vec3 p = vec3(in_uv.x,waterDataC.y,in_uv.y);
		float NOISE_STRENGTH = min( waterDataC.y * 0.5f, 1.0f);
		float CHOPPY = min( waterDataC.y * 0.25f, 1.0f );
		float noiseC = map_detailed(p,CHOPPY) * NOISE_STRENGTH;
		float noiseL = map_detailed(p - vec3(texelSize.x,0,0), CHOPPY) * NOISE_STRENGTH;
		float noiseR = map_detailed(p + vec3(texelSize.x,0,0), CHOPPY) * NOISE_STRENGTH;
		float noiseU = map_detailed(p - vec3(0,0,texelSize.y), CHOPPY) * NOISE_STRENGTH;
		float noiseD = map_detailed(p + vec3(0,0,texelSize.y), CHOPPY) * NOISE_STRENGTH;


		float heightR = rockDataR.x + rockDataR.y + rockDataR.w + waterDataR.x + waterDataR.y + noiseR;
		float heightL = rockDataL.x + rockDataL.y + rockDataL.w + waterDataL.x + waterDataL.y + noiseL;
		float heightU = rockDataU.x + rockDataU.y + rockDataU.w + waterDataU.x + waterDataU.y + noiseU;
		float heightD = rockDataD.x + rockDataD.y + rockDataD.w + waterDataD.x + waterDataD.y + noiseD;
		float heightC = rockDataC.x + rockDataC.y + rockDataC.w + waterDataC.x + waterDataC.y + noiseC;
		
		vec3 va = normalize(vec3(u_cellSize.x, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y));
		waterNormal = -cross(va,vb);
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Foam
	//////////////////////////////////////////////////////////////////////////////////
	{
		// Generate a map where the tips of waves are white (generate foam)
		vec4 waterDataC = textureLod(s_waterData, in_uv, 0);
		vec4 rockDataC = textureLod(s_rockData, in_uv, 0);

		float height = rockDataC.x + rockDataC.y + rockDataC.w + waterDataC.x + waterDataC.y;

		float heightDifferenceTotal = 0.0f;
		float strength = 80.0f;
		for ( int i = 1; i < 7; i++ )
		{
			vec4 waterDataM = textureLod(s_waterData, in_uv, i);
			vec4 rockDataM = textureLod(s_rockData, in_uv, i);

			float heightM = rockDataM.x + rockDataM.y + rockDataM.w + waterDataM.x + waterDataM.y;

			float diff = max(height - heightM, 0);

			heightDifferenceTotal += diff * strength;

			strength *= 0.9;
		}


		mappingDataC.w = heightDifferenceTotal;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Calculate rock normals
	//////////////////////////////////////////////////////////////////////////////////
	vec3 rockNormal;
	{
		float heightR = rockDataR.x + rockDataR.y + rockDataR.w + mappingDataR.y * u_mapHeightOffset;
		float heightL = rockDataL.x + rockDataL.y + rockDataL.w + mappingDataL.y * u_mapHeightOffset;
		float heightU = rockDataU.x + rockDataU.y + rockDataU.w + mappingDataU.y * u_mapHeightOffset;
		float heightD = rockDataD.x + rockDataD.y + rockDataD.w + mappingDataD.y * u_mapHeightOffset;
		float heightC = rockDataC.x + rockDataC.y + rockDataC.w + mappingDataC.y * u_mapHeightOffset;

		vec3 va = normalize(vec3(u_cellSize.x, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y));
		rockNormal = -cross(va,vb);
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Calculate occlusion
	//////////////////////////////////////////////////////////////////////////////////
	float occlusion = 0.0f;
	for ( int i = 1; i < u_numHeightMips; i++ )
	{
		float heightC = rockDataC.x + rockDataC.y + rockDataC.w;// + mappingDataC.y * u_mapHeightOffset;

		vec4 mippedRockDataC = textureLod(s_rockData, in_uv, float(i));
		float mippedHeight = mippedRockDataC.x + mippedRockDataC.y + mippedRockDataC.a;
		float diff = max(0.0f, mippedHeight - heightC);
		float ratio = diff / u_cellSize.x;
		float angle = atan(ratio);
		const float PI = 3.14159265359;
		const float HALF_PI = PI * 0.5f;
		float occlusionFoThisMip = angle / HALF_PI;

		occlusion += occlusionFoThisMip;
	}
	occlusion /= u_numHeightMips;

	
	// Output
	out_mappingData = mappingDataC;
	out_rockNormal = vec4(rockNormal,occlusion);
	out_waterNormal = vec4(waterNormal,0.0f);
}








