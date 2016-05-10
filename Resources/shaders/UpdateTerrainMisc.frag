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

uniform float u_moltenViscosity;
uniform float u_rockMeltingPoint;

uniform float u_waterViscosity;
uniform float u_mouseRadius;
uniform float u_mouseVolumeStrength;
uniform float u_mouseHeatStrength;

uniform vec2 u_cellSize;

uniform int u_numHeightMips;

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

float CalcMoltenViscosity( float _heat )
{
	return smoothstep( 0.0f, 1.0f, clamp(_heat-u_rockMeltingPoint, 0.0f, 1.0f)) * u_moltenViscosity;
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
	float viscosity = CalcMoltenViscosity(heat);

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
		newPhase += min(u_cycleSpeed * seperationAmount, 0.1f);
		newPhase = mod(newPhase, 1.0f);

		float samplePos = newPhase;
		float newBump = texture(s_diffuseMap, vec2(samplePos,0)).x;
		
		mappingDataC.x = newPhase;
		mappingDataC.y = newBump;
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Calculate water normal
	//////////////////////////////////////////////////////////////////////////////////
	vec3 waterNormal;
	{
		float noiseC = waterDataC.w;
		float noiseL = waterDataL.w;
		float noiseR = waterDataR.w;
		float noiseU = waterDataU.w;
		float noiseD = waterDataD.w;

		float heightR = rockDataR.x + rockDataR.y + rockDataR.w + waterDataR.x + noiseR;
		float heightL = rockDataL.x + rockDataL.y + rockDataL.w + waterDataL.x + noiseL;
		float heightU = rockDataU.x + rockDataU.y + rockDataU.w + waterDataU.x + noiseU;
		float heightD = rockDataD.x + rockDataD.y + rockDataD.w + waterDataD.x + noiseD;
		float heightC = rockDataC.x + rockDataC.y + rockDataC.w + waterDataC.x + noiseC;
		
		vec3 va = normalize(vec3(u_cellSize.x, heightR-heightL, 0.0f));
		vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y));
		waterNormal = -cross(va,vb);
	}
	
	//////////////////////////////////////////////////////////////////////////////////
	// Calculate rock normals
	//////////////////////////////////////////////////////////////////////////////////
	vec3 rockNormal;
	{
		float heightR = rockDataR.x + rockDataR.y + rockDataR.w;
		float heightL = rockDataL.x + rockDataL.y + rockDataL.w;
		float heightU = rockDataU.x + rockDataU.y + rockDataU.w;
		float heightD = rockDataD.x + rockDataD.y + rockDataD.w;
		float heightC = rockDataC.x + rockDataC.y + rockDataC.w;

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
		float heightC = rockDataC.x + rockDataC.y + rockDataC.w;

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








