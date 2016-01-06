#version 430 core

// Samplers
uniform sampler2D s_heightData;
uniform sampler2D s_mappingData;
uniform sampler2D s_diffuseMap;
uniform sampler2D s_fluxData;

// Inputs
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
layout( location = 0 ) out vec4 out_velocityData;
layout( location = 1 ) out vec4 out_mappingData;
layout( location = 2 ) out vec4 out_normal;

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

void main(void)
{
	ivec2 dimensions = textureSize( s_heightData, 0 );
	vec2 texelSize = 1.0f / dimensions;

	ivec2 texelCoordC = ivec2(gl_FragCoord.xy);
	ivec2 texelCoordL = texelCoordC - ivec2(1,0);
	ivec2 texelCoordR = texelCoordC + ivec2(1,0);
	ivec2 texelCoordU = texelCoordC - ivec2(0,1);
	ivec2 texelCoordD = texelCoordC + ivec2(0,1);
	ivec2 texelCoordUL = texelCoordC - ivec2(1,1);
	ivec2 texelCoordUR = texelCoordC + ivec2(-1,1);
	ivec2 texelCoordDL = texelCoordC + ivec2(1,-1);
	ivec2 texelCoordDR = texelCoordC + ivec2(1,1);

	vec4 heightDataC = texelFetch(s_heightData, texelCoordC, 0);
	vec4 heightDataL = texelFetch(s_heightData, texelCoordL, 0);
	vec4 heightDataR = texelFetch(s_heightData, texelCoordR, 0);
	vec4 heightDataU = texelFetch(s_heightData, texelCoordU, 0);
	vec4 heightDataD = texelFetch(s_heightData, texelCoordD, 0);

	vec4 mappingDataC = texelFetch(s_mappingData, texelCoordC, 0);
	vec4 mappingDataL = texelFetch(s_mappingData, texelCoordL, 0);
	vec4 mappingDataR = texelFetch(s_mappingData, texelCoordR, 0);
	vec4 mappingDataU = texelFetch(s_mappingData, texelCoordU, 0);
	vec4 mappingDataD = texelFetch(s_mappingData, texelCoordD, 0);

	vec4 fluxC  = texelFetch(s_fluxData, texelCoordC, 0);
	vec4 fluxL  = texelFetch(s_fluxData, texelCoordL, 0);
	vec4 fluxR  = texelFetch(s_fluxData, texelCoordR, 0);
	vec4 fluxU  = texelFetch(s_fluxData, texelCoordU, 0);
	vec4 fluxD  = texelFetch(s_fluxData, texelCoordD, 0);
	vec4 fluxUL = texelFetch(s_fluxData, texelCoordUL, 0);
	vec4 fluxUR = texelFetch(s_fluxData, texelCoordUR, 0);
	vec4 fluxDL = texelFetch(s_fluxData, texelCoordDL, 0);
	vec4 fluxDR = texelFetch(s_fluxData, texelCoordDR, 0);

	vec4 diffuseSampleC = texture(s_diffuseMap, in_uv);

	float heat = heightDataC.z;
	float viscosity = CalcViscosity(heat, mappingDataC.y);

	// Calculate velocity from flux
	vec4 velocity = vec4(0.0f);
	velocity.x = ((fluxL.y + fluxC.y) - (fluxR.x + fluxC.x)) / texelSize.x;
	velocity.y = ((fluxU.w + fluxC.w) - (fluxD.z + fluxC.z)) / texelSize.y;
	velocity.xy *= u_velocityScalar * viscosity;

	
	vec4 heightDiffs = vec4(0.0f);
	heightDiffs.x = (heightDataL.x+heightDataL.y) - (heightDataC.x+heightDataC.y);
	heightDiffs.y = (heightDataR.x+heightDataR.y) - (heightDataC.x+heightDataC.y);
	heightDiffs.z = (heightDataU.x+heightDataU.y) - (heightDataC.x+heightDataC.y);
	heightDiffs.w = (heightDataD.x+heightDataD.y) - (heightDataC.x+heightDataC.y);

	vec4 h = vec4(0.0f);
	h.x = length( vec2(heightDiffs.x, u_cellSize.x) );
	h.y = length( vec2(heightDiffs.y, u_cellSize.x) );
	h.z = length( vec2(heightDiffs.z, u_cellSize.y) );
	h.w = length( vec2(heightDiffs.w, u_cellSize.y) );
	velocity.xy *= (1.0f + (h.x+h.y+h.z+h.w));
	

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
		vec4 mippedHeightDataC = textureLod(s_heightData, in_uv, float(i));
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
	out_velocityData = velocity;
	out_mappingData = mappingDataC;
	out_normal = vec4(normal,occlusion);
}








