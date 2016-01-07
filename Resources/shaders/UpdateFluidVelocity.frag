#version 430 core

////////////////////////////////////////////////////////////////
// Inputs
////////////////////////////////////////////////////////////////

// Samplers
uniform sampler2D s_rockData;
uniform sampler2D s_rockFluxData;
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
uniform float u_velocityScalar;

uniform float u_viscosityMin;
uniform float u_viscosityMax;
uniform float u_heatViscosityPower;
uniform float u_heatViscosityBias;

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
	velocity *= u_velocityScalar * viscosity;

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

	vec4 heightDataC = texelFetch(s_rockData, texelCoordC, 0);
	vec4 heightDataL = texelFetch(s_rockData, texelCoordL, 0);
	vec4 heightDataR = texelFetch(s_rockData, texelCoordR, 0);
	vec4 heightDataU = texelFetch(s_rockData, texelCoordU, 0);
	vec4 heightDataD = texelFetch(s_rockData, texelCoordD, 0);

	vec4 mappingDataC = texelFetch(s_mappingData, texelCoordC, 0);
	vec4 mappingDataL = texelFetch(s_mappingData, texelCoordL, 0);
	vec4 mappingDataR = texelFetch(s_mappingData, texelCoordR, 0);
	vec4 mappingDataU = texelFetch(s_mappingData, texelCoordU, 0);
	vec4 mappingDataD = texelFetch(s_mappingData, texelCoordD, 0);

	vec4 fluxC  = texelFetch(s_rockFluxData, texelCoordC, 0);
	vec4 fluxL  = texelFetch(s_rockFluxData, texelCoordL, 0);
	vec4 fluxR  = texelFetch(s_rockFluxData, texelCoordR, 0);
	vec4 fluxU  = texelFetch(s_rockFluxData, texelCoordU, 0);
	vec4 fluxD  = texelFetch(s_rockFluxData, texelCoordD, 0);

	float heat = heightDataC.z;
	float viscosity = CalcViscosity(heat, mappingDataC.y);

	vec4 velocity = vec4( VelocityFromFlux(fluxC, fluxL, fluxR, fluxU, fluxD, texelSize, viscosity ), 0.0f, 0.0f );

	//////////////////////////////////////////////////////////////////////////////////
	// Update Mapping
	// Visit each 9 neighbours, and determine which ones will sample from within this.
	// cell. Those that do exert a 'pull' on this cell from their direction. If this 
	// pull is counteracted by another neighbour in the opposite direction then this 
	// cell is being 'seperated' and should cycle its sampled colour so that 
	// neighbours pulling this colour get fresh detail, rather than smearing a single 
	// value
	//////////////////////////////////////////////////////////////////////////////////
	vec2 pullUV = in_uv - velocity.xy * u_textureScrollSpeed * texelSize;
	vec4 pulledValue = texture(s_mappingData, pullUV);

	mappingDataC.x = clamp( pulledValue.x, 0.0f, 1.0f );
	
	vec4 neighbourFlux[4] = vec4[]( fluxU, fluxR, fluxD, fluxL );
	vec2 neighbourOffsets[4] = vec2[]( vec2(0,-1), vec2(1,0), vec2(0,1), vec2(-1,0) );
	float seperationAmount = 0.0f;
	for ( int i = 0; i < 4; i++ )
	{
		vec4 fluxN = neighbourFlux[i];
		vec2 offset = neighbourOffsets[i];

		vec2 velocityN = vec2(fluxN.y-fluxN.x, fluxN.w-fluxN.z);
		velocityN *= u_velocityScalar * viscosity;
		vec2 relativeVelocityN = velocityN - velocity.xy;

		if ( length(relativeVelocityN) < 0.001f ) 
			continue;

		float dp = dot( normalize(relativeVelocityN), normalize(offset) );
		seperationAmount += dp * length(relativeVelocityN);
	}
	seperationAmount = max(seperationAmount, 0.0f);

	// Insert some extra seperation if the mouse is adding volume here
	vec2 mousePos = GetMousePos();
	float mouseRatio = 1.0f - min(1.0f, length(in_uv-mousePos) / u_mouseRadius);
	mouseRatio = pow(mouseRatio, 1.5f);
	//seperationAmount += (mouseRatio * u_mouseVolumeStrength) * u_cycleSpeed;

	float cyclePhase = mappingDataC.x + min(u_cycleSpeed * seperationAmount,0.1f);
	cyclePhase = mod(cyclePhase, 1.0f);
	mappingDataC.x = cyclePhase;

	float diffuseSampleA = texture(s_diffuseMap, in_uv + velocity.xy * u_textureScrollSpeed * 0.25f).y;
	float samplePos = pulledValue.x - (1.0f-diffuseSampleA) * (1.0f - clamp((heat-u_heatViscosityBias)*0.5f, 0.0f, 1.0f)) * 0.05f;
	float targetValue = texture(s_diffuseMap, vec2(samplePos,0)).x;
	mappingDataC.y = targetValue;

	//////////////////////////////////////////////////////////////////////////////////
	// Calculate normals
	//////////////////////////////////////////////////////////////////////////////////
	float heightR = heightDataR.x + heightDataR.y + mappingDataR.y * u_mapHeightOffset;
	float heightL = heightDataL.x + heightDataL.y + mappingDataL.y * u_mapHeightOffset;
	float heightU = heightDataU.x + heightDataU.y + mappingDataU.y * u_mapHeightOffset;
	float heightD = heightDataD.x + heightDataD.y + mappingDataD.y * u_mapHeightOffset;
	float heightC = heightDataC.x + heightDataC.y + mappingDataC.y * u_mapHeightOffset;

	vec3 va = normalize(vec3(u_cellSize.x, heightR-heightL, 0.0f));
    vec3 vb = normalize(vec3(0.0f, heightD-heightU, u_cellSize.y));
    vec3 normal = -cross(va,vb);

	//////////////////////////////////////////////////////////////////////////////////
	// Calculate occlusion
	//////////////////////////////////////////////////////////////////////////////////
	float occlusion = 0.0f;
	for ( int i = 1; i < u_numHeightMips; i++ )
	{
		vec4 mippedHeightDataC = textureLod(s_rockData, in_uv, float(i));
		float mippedHeight = mippedHeightDataC.x + mippedHeightDataC.y;
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
	out_rockNormal = vec4(normal,occlusion);
}








