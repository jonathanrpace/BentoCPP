//-----------------------------------------------------------
// Outputs
//-----------------------------------------------------------
layout( location = 0 ) out vec4 out_bounceAndAO;
layout( location = 1 ) out vec4 out_reflections;

//-----------------------------------------------------------
// Inputs
//-----------------------------------------------------------
// TAU_LOOKUP[N-1] = optimal number of spiral turns for N samples
const int TAU_LOOKUP[ ] = 
{
	1, 1, 2, 3, 2, 5, 2, 3, 2, 3, 3, 5, 5, 3, 4, 7, 5, 5, 7, 9, 8, 5, 5, 7, 7, 7, 8, 5, 8, 11, 12, 7, 10, 13, 8, 11,
	8, 7, 14, 11, 11, 13, 12, 13, 19, 17, 13, 11, 18, 19, 11, 11, 14, 17, 21, 15, 16, 17, 18, 13, 17, 11, 17, 19,
	18, 25, 18, 19, 19, 29, 21, 19, 27, 31, 29, 21,	18, 17, 29, 31, 31, 23, 18, 25, 26, 25, 23, 19, 34, 19, 27,	21, 
	25, 39, 29, 17, 21, 27
};
const float PI = 3.142f;
const int NUM_SAMPLES = 12;
const float TAU = TAU_LOOKUP[NUM_SAMPLES] * PI * 2.0f;
const float NUM_SAMPLES_RCP = 1.0f / NUM_SAMPLES;
const float Q = 0.25f;			// The screen space radius as which we begin dropping mips
uniform float BIAS = 0.06;		// The offset applied to minimise self-occlusion.
const float EPSILON = 0.003f;	// A small offset to avoid divide by zero

const int MAX_REFLECTION_STEPS = 24;
const int NUM_BINARY_SERACH_STEPS = 0;
uniform float RAY_STEP_SCALAR = 1.0f;
uniform float RAY_STEP = 0.02f;
const float MAX_RAY_LENGTH = RAY_STEP * MAX_REFLECTION_STEPS;
const int NUM_REFLECTION_SAMPLES = 2;

uniform sampler2D s_randomTexture;
uniform sampler2DRect s_positionBufferRect;
uniform sampler2DRect s_prevPositionBufferRect;
uniform sampler2DRect s_normalBufferRect;
uniform sampler2DRect s_directLightBufferRect;
uniform sampler2DRect s_indirectLightBufferRect;
uniform sampler2DRect s_prevBounceAndAo;
uniform sampler2DRect s_prevReflection;
uniform sampler2DRect s_material;					// Not yet small
uniform sampler2DRect s_albedo;						// Not yet small

//uniform int u_maxMip;

uniform mat4 u_projectionMatrix;
uniform mat4 u_invViewMatrix;
uniform mat4 u_prevViewProjectionMatrix;
uniform mat4 u_prevInvViewProjectionMatrix;

uniform float u_roughnessJitterMin = 0.001f;
uniform float u_roughnessJitterMax = 0.1f;
uniform float u_radius = 0.5f;
uniform float u_aspectRatio;
uniform float u_radiosityScalar = 10.0f;
uniform float u_aoScalar = 3.0f;
uniform float u_aoAttenutationPower = 2.0f;
uniform float u_bounceAttenutationPower = 2.0f;
uniform float u_colorBleedingBoost = 0.8f;
uniform float u_lightTransportResolutionScalar;
uniform float u_maxReflectDepthDiff;
uniform float u_nominalReflectDepthDiff = 0.1f;
uniform float u_time;
uniform float SAMPLE_MIX_RATE = 0.001f;
uniform float SAMPLE_MIX_RATE_REFLECTIONS = 0.25f;
uniform bool u_flag = true;
uniform float u_multiBounceScalar = 1.0f;
uniform int u_averageSpan = 30;

// Global vars
ivec2 frameBufferSize;

// Inputs
in Varying
{
	in vec2 in_uv;
};

//-----------------------------------------------------------
// Functions
//-----------------------------------------------------------

vec4 RayCast
(
	vec3 dir, 
	vec3 startPos, 
	out vec2 bestHitUV, 
	out vec3 bestPosition
)
{
	vec4 pointAlongRay = vec4(startPos,1.0f);
    vec4 projectedCoord;
    vec4 samplePos;
    float rayStep = RAY_STEP;// * (1.0f+abs(dir.z));

    for(int i = 0; i < MAX_REFLECTION_STEPS; i++)
    {
    	vec3 offset = dir * rayStep;
    	//rayStep *= RAY_STEP_SCALAR;
		
		pointAlongRay.xyz += offset;
		projectedCoord = u_projectionMatrix * pointAlongRay;
		projectedCoord.xyz /= projectedCoord.w;

		if ( 	   projectedCoord.x < -1 
				|| projectedCoord.y < -1 
				|| projectedCoord.z < 0 
				|| projectedCoord.x > 1 
				|| projectedCoord.y > 1
				|| projectedCoord.z > 1 )

		{
			return vec4(0.0f);
		}

		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
		vec2 projectedCoordRect = projectedCoord.xy * frameBufferSize;
		samplePos = texture( s_positionBufferRect, projectedCoordRect );
		float depthDiff = length( pointAlongRay.xyz ) - length( samplePos.xyz );

		if ( depthDiff > 0.0f && depthDiff < u_nominalReflectDepthDiff )
		{
			bestHitUV = projectedCoord.xy;
			bestPosition = pointAlongRay.xyz;

			return vec4(1.0f);
		}
	}

	return vec4(0.0f);

}

void main()
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Shared values
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	frameBufferSize = textureSize( s_positionBufferRect, 0 );
	vec2 scaledFragCoord = gl_FragCoord.xy / u_lightTransportResolutionScalar;
	vec2 fragCoord = gl_FragCoord.xy;
	vec4 material = texture( s_material, scaledFragCoord );
	float roughness = material.x;
	float reflectivity = material.y;
	vec3 albedo = texture( s_albedo, scaledFragCoord ).rgb;
	vec3 fragPos = texture( s_positionBufferRect, fragCoord ).xyz;
	vec3 fragNormal = normalize( texture( s_normalBufferRect, fragCoord ).xyz );
	ivec2 randomTextureSize = textureSize( s_randomTexture, 0 );
	vec4 randomSample = texture( s_randomTexture, (fragCoord / randomTextureSize) );
	float randomNumber = randomSample.w;
	vec3 viewDir = normalize( -fragPos );
	float dotNV = clamp( dot( viewDir, fragNormal ), 0.0f, 1.0f );
	//float bias = BIAS * -fragPos.z;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// AO and Bounce
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	float offset = (randomNumber + u_time) * PI * 2.0f;
	float aoTotal = 0.0f;
	vec3 bounceTotal = vec3(0.0f, 0.0f, 0.0f);
	vec3 prominantBounceDirection = vec3(0.0f);
	vec3 prominantAODirection = vec3(0.0f);
	for ( int i = 0; i < NUM_SAMPLES; i++ )
	{
		// The ratio along the spiral
		float theta = NUM_SAMPLES_RCP * (i + 0.25f);
		float currAngle = offset + theta * TAU;

		// Distance away from frag UV
		float h = abs( u_radius * theta + randomSample.x * u_radius * 0.1f );
		h /= length(1.0f + fragPos);
		
		// UV offset away from frag UV
		vec2 offset = vec2( cos( currAngle ), sin( currAngle ) * u_aspectRatio );
		vec2 sampleUV = in_uv + offset * h;

		//float mipLevel = min( log2( h / Q ), u_maxMip );
		vec2 sampleRectUV = sampleUV * frameBufferSize;
		vec3 samplePosition = texture( s_positionBufferRect, sampleRectUV ).xyz;
		vec3 sampleDirectColor = texture( s_directLightBufferRect, sampleRectUV ).xyz;
		vec3 sampleIndirectColor = texture( s_indirectLightBufferRect, sampleRectUV ).xyz;
		vec3 sampleMaterial = texture( s_material, sampleUV * frameBufferSize / u_lightTransportResolutionScalar ).xyz;
		float sampleRoughness = sampleMaterial.x;
		float sampleEmissive = sampleMaterial.z;
		vec3 sampleColor = sampleDirectColor + sampleIndirectColor + (sampleIndirectColor * sampleEmissive * 1.0f);

		// Add the bounced light from the previous frame
		vec3 sampleBounce = texture( s_prevBounceAndAo, sampleRectUV ).rgb;
		//sampleColor += sampleBounce * u_multiBounceScalar;

		// Calculate distance and direction between fragment and sample pos
		vec3 v = samplePosition - fragPos;
		float vLength = length(v);
		v = normalize(v);

		// Determine common value for AO and bounce
		float aoAttenuation = 1.0f / (1.0f + pow( vLength, u_aoAttenutationPower ));
		float bounceAttenuation = 1.0f / (1.0f + pow( vLength, u_bounceAttenutationPower ));

		float vDotFragNormal = clamp( dot( v, fragNormal ), 0.0f, 1.0f );
		float vDotFragNormalBiased = clamp( dot( v, fragNormal ) - BIAS, 0.0f, 1.0f );

		// Sum AO
		float aoAmount = aoAttenuation * vDotFragNormalBiased;
		aoTotal += aoAmount;

		// Sum Bounce
		vec3 bounceAmount = sampleColor * vDotFragNormalBiased * bounceAttenuation * albedo;

		// Fade off bounce light from shiny surfaces, as their color is view dependant.
		bounceAmount *= pow( sampleRoughness, 0.5f );
		bounceTotal += bounceAmount;

		// Track prominant ao/bounce direction
		prominantAODirection -= v * aoAmount;
		float maxBounce = max( max( bounceAmount.r, bounceAmount.g ), bounceAmount.b );
		prominantBounceDirection -= v * maxBounce;
		// Push back the prominant AO direction if it's being occluded by the thing illuminating it.
		prominantAODirection += v * maxBounce;
	}

	// Normalise AO
	aoTotal *= NUM_SAMPLES_RCP;
	//aoTotal *= roughness;
	aoTotal = min( 1.0f, aoTotal * u_aoScalar );

	// Normalise bounce
	bounceTotal *= NUM_SAMPLES_RCP;
	bounceTotal *= u_radiosityScalar;

	// Boost colour bleeding
	float maxBounceChannel = max( max( bounceTotal.r, bounceTotal.g ), bounceTotal.b );
	float minBounceChannel = min( min( bounceTotal.r, bounceTotal.g ), bounceTotal.b );
	float colorBoost = max( 0.0f, (maxBounceChannel - minBounceChannel) / maxBounceChannel );
	colorBoost = mix( 1.0f, colorBoost, u_colorBleedingBoost );
	bounceTotal * colorBoost;

	// Apply fresnel to bounce
	float F = Fresnel( dotNV, reflectivity );
	bounceTotal *= F;

	bounceTotal *= pow( roughness, 0.5f );
	bounceTotal *= albedo;

	// Try to do modulate the bouncelight by the ao in a fancy way.
	// We've been keeping track of which direction the fragment is being occluded by.
	// And also keeping track of the rough direction it's being illuminated by bounce light.
	// If these don't dp3() up, it means the fragment is mostly being occluded in a direction away from
	// the bounce illumination, so we should introduce some of the ao term to the bounce.
	float prominantAOStrength = max( 1.0f, length( prominantAODirection * 0.5f ) );
	float prominantBounceStrength = max( 1.0f, length( prominantBounceDirection * 0.5f ) );
	float prominantDot = clamp( dot( normalize(prominantAODirection), normalize(prominantBounceDirection) ), 0.0f, 1.0f );
	bounceTotal = mix( bounceTotal, bounceTotal * (1.0f-aoTotal), prominantDot );
	//bounceTotal -= bounceTotal * prominantDot * prominantAOStrength * prominantBounceStrength * aoTotal;

	bounceTotal = normalize((prominantAODirection+1.0f)*0.5f);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Reflections
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	vec3 reflectionColor = vec3(0.0f,0.0f,0.0f);
	float reflectionAlpha = 0.0f;
	vec3 reflectionFragNormal = fragNormal;

	for ( int i = 0; i < NUM_REFLECTION_SAMPLES; i++ )
	{
		vec2 randomUV = (gl_FragCoord.xy / randomTextureSize) + randomSample.xy + randomSample.xy * u_time;
		//randomUV += u_time;

		vec3 randomSampleReflect = texture( s_randomTexture, randomUV ).xyz;
		vec3 randomDirection = randomSampleReflect.xyz;
		reflectionFragNormal += randomDirection * (u_roughnessJitterMin + pow(roughness,1.5f) * u_roughnessJitterMax);
		reflectionFragNormal = normalize(reflectionFragNormal);

		vec3 reflectVec = reflect( normalize( fragPos ), reflectionFragNormal ); 

		vec2 hitUV;					// Output
		vec3 hitPos;				// Output

		vec4 success = RayCast(reflectVec, fragPos, hitUV, hitPos);

		vec2 hitRectUV = hitUV * frameBufferSize;
		reflectionColor += texture(s_directLightBufferRect, hitRectUV).rgb;
		reflectionColor += texture(s_indirectLightBufferRect, hitRectUV).rgb;
		reflectionColor += texture(s_prevBounceAndAo, hitRectUV).rgb;

		// Now we have a ray intersection, and we've got our color/normal sample.
		// Time to find all the things wrong with this sample, and modulate it to make its
		// shortcomings less obvious.
		float reflectionAlphaCurrent = success.x;

		// The ray will eventually just stop, and we'd like to fade a bit before it does so
		// there's no hard edge past a certain ray distance.
		float distanceStrength = 1.0f - min( length(hitPos.xyz - fragPos.xyz) / MAX_RAY_LENGTH, 1.0f );
		distanceStrength = pow(distanceStrength, 0.5f);
		reflectionAlphaCurrent *= distanceStrength;

		// Reflection rays pointing towards the camere are suspect, as we have no 'back face' information
		float backfaceFalloff = clamp( dot( vec3(0.0f,0.0f,1.0f), -reflectVec ), 0.0f, 1.0f );
		backfaceFalloff = pow(backfaceFalloff, 0.5f);
		reflectionAlphaCurrent *= backfaceFalloff;

		// Multiply by fresnel
		vec3 halfVector = normalize(viewDir + reflectVec);
		float dotLH = clamp(dot(reflectVec,halfVector), 0.0f, 1.0f);
		float F = Fresnel( dotNV, reflectivity );
		reflectionAlphaCurrent *= F;

		reflectionAlpha += reflectionAlphaCurrent;
	}
	reflectionColor /= NUM_REFLECTION_SAMPLES;
	reflectionAlpha /= NUM_REFLECTION_SAMPLES;

	// Non-shiny things shouldn't reflect
	reflectionAlpha *= (1.0f-roughness*roughness*roughness);
	reflectionColor *= albedo;

	// Retreive samples from the previous frame via reprojection
	vec4 fragWorldPos = u_invViewMatrix * vec4(fragPos,1.0);
	vec4 fragProjectedPos = u_projectionMatrix * vec4(fragPos,1.0);
	fragProjectedPos.xyz /= fragProjectedPos.w;

	vec4 prevFragProjectedPos = u_prevViewProjectionMatrix * fragWorldPos;
	prevFragProjectedPos.xyz /= prevFragProjectedPos.w;

	vec2 prevUV = (prevFragProjectedPos.xy + 1.0) * 0.5;
	vec2 prevRectUV = prevUV * frameBufferSize;
	vec4 prevFragWorldPos = u_invViewMatrix * vec4( texture( s_prevPositionBufferRect, prevRectUV ).xyz, 1.0f );

	prevFragWorldPos.w = 1.0f;
	prevFragProjectedPos = u_prevViewProjectionMatrix * prevFragWorldPos;
	prevFragProjectedPos.xyz /= prevFragProjectedPos.w;

	float dis = abs(fragProjectedPos.z - prevFragProjectedPos.z);
	//float dis = length(prevFragWorldPos.xyz-fragWorldPos.xyz);
	if ( false )//dis < 0.00005f  )
	{
		vec4 prevBounceAndAO = texture( s_prevBounceAndAo, prevRectUV );

		float prevAOTotal= 1.0f - prevBounceAndAO.w;
		aoTotal = (prevAOTotal * u_averageSpan + aoTotal) / (u_averageSpan+1);
		//aoTotal = mix( prevAOTotal, aoTotal, SAMPLE_MIX_RATE );

		vec3 prevBounceTotal = prevBounceAndAO.rgb;
		bounceTotal = (prevBounceTotal * u_averageSpan + bounceTotal) / (u_averageSpan+1);
		//bounceTotal = mix( prevBounceTotal, bounceTotal, vec3(SAMPLE_MIX_RATE) );

		vec4 prevReflectionColorAndAlpha = texture( s_prevReflection, prevRectUV );
		reflectionColor = mix( prevReflectionColorAndAlpha.rgb, reflectionColor, vec3(SAMPLE_MIX_RATE_REFLECTIONS) );
		reflectionAlpha = mix( prevReflectionColorAndAlpha.a, reflectionAlpha, SAMPLE_MIX_RATE_REFLECTIONS );
	}

	aoTotal = 1.0f - aoTotal;

	if ( true )
	{
		out_bounceAndAO = vec4( bounceTotal, aoTotal );
	}
	else
	{
		out_bounceAndAO = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	}

	out_reflections = vec4( reflectionColor, reflectionAlpha );
}
