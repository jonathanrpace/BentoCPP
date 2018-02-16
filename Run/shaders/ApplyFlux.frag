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
uniform sampler2D s_moltenFluxData;
uniform sampler2D s_waterFluxData;
uniform sampler2D s_miscData;

uniform float u_moltenDiffuseStrength;
uniform float u_advectSpeed = 1.0;
uniform float u_dirtTransportSpeed = 1.0;

const float DT = 1.0 / 6.0;

// Outputs
layout( location = 0 ) out vec4 out_heightData;
layout( location = 1 ) out vec4 out_miscData;


////////////////////////////////////////////////////////////////
//
void getFlux
(
	in  sampler2D	i_sampler,
	out vec4		o_fluxC,
	out vec4		o_fluxN
)
{
	ivec2 T = ivec2(gl_FragCoord.xy);

	o_fluxC = texelFetch(i_sampler, T, 0);
	o_fluxN = vec4(
		texelFetchOffset(i_sampler, T, 0, ivec2( 0,-1)).w,
		texelFetchOffset(i_sampler, T, 0, ivec2( 0, 1)).z,
		texelFetchOffset(i_sampler, T, 0, ivec2( 1, 0)).x,
		texelFetchOffset(i_sampler, T, 0, ivec2(-1, 0)).y
	);
}

////////////////////////////////////////////////////////////////
//
void getVolumeTransportVars
( 
	in  float i_volumeC,
	in  vec4  i_volumeN,
	in  vec4  i_fluxC, 
	in  vec4  i_fluxN,
	out float o_totalExported, 
	out float o_totalImported, 
	out float o_exportedProp, 
	out vec4  o_importedProp )
{
	vec4 exported = min( i_fluxC.zwyx, i_volumeC.xxxx ) * DT;
	o_totalExported = dot( exported, vec4(1.0) );

	o_exportedProp = min( 1.0, o_totalExported / i_volumeC );
	o_exportedProp = isnan(o_exportedProp) || isinf(o_exportedProp) ? 1.0 : o_exportedProp;

	vec4 imported = min( i_fluxN, i_volumeN ) * DT;
	o_totalImported = dot( imported, vec4(1.0) );

	o_importedProp = min( vec4(1.0), imported / i_volumeN );
	bvec4 nan = isnan(o_importedProp);
	bvec4 inf = isinf(o_importedProp);
	o_importedProp.x = nan.x || inf.x ? 1.0 : o_importedProp.x;
	o_importedProp.y = nan.y || inf.y ? 1.0 : o_importedProp.y;
	o_importedProp.z = nan.z || inf.z ? 1.0 : o_importedProp.z;
	o_importedProp.w = nan.y || inf.w ? 1.0 : o_importedProp.w;
}

////////////////////////////////////////////////////////////////
//
vec4 advect
(
	in sampler2D	i_valuesSampler,
	in float		i_exportedProp,
	in vec4			i_importedProp,
	float			i_advectSpeed
)
{
	ivec2 T = ivec2(gl_FragCoord.xy);

	vec4 valuesC = texelFetch( i_valuesSampler, T, 0 );
	vec4 valuesN = texelFetchOffset(i_valuesSampler, T, 0, ivec2( 0,-1));
	vec4 valuesS = texelFetchOffset(i_valuesSampler, T, 0, ivec2( 0, 1));
	vec4 valuesE = texelFetchOffset(i_valuesSampler, T, 0, ivec2( 1, 0));
	vec4 valuesW = texelFetchOffset(i_valuesSampler, T, 0, ivec2(-1, 0));
	
	vec4 valuesOut = valuesC;

	valuesOut -= i_exportedProp * valuesC * i_advectSpeed;
	valuesOut += ((i_importedProp.x * valuesN) + (i_importedProp.y * valuesS) + (i_importedProp.z * valuesE) + (i_importedProp.w * valuesW)) * i_advectSpeed;

	return valuesOut;
}

////////////////////////////////////////////////////////////////
//
void main(void)
{ 
	ivec2 T = ivec2(gl_FragCoord.xy);
	
	vec4 hC = texelFetch(s_heightData, T, 0);
	vec4 hN = texelFetchOffset(s_heightData, T, 0, ivec2( 0,-1));
	vec4 hS = texelFetchOffset(s_heightData, T, 0, ivec2( 0, 1));
	vec4 hE = texelFetchOffset(s_heightData, T, 0, ivec2( 1, 0));
	vec4 hW = texelFetchOffset(s_heightData, T, 0, ivec2(-1, 0));
	vec4 miscC = texelFetch(s_miscData, T, 0);
	
	float solidHeight = hC.x;
	float moltenHeight = hC.y;
	float dirtHeight = hC.z;
	float waterHeight = hC.w;

	//////////////////////////////
	// Molten
	//////////////////////////////
	{
		// Diffuse volume
		{
			vec4 heightC = vec4( solidHeight + moltenHeight );
			vec4 heightN = vec4( hW.x + hW.y, hE.x + hE.y, hN.x + hN.y, hS.x + hS.y );
			vec4 diffs = heightN - heightC;
		
			vec4 clampedDiffs = clamp( diffs * 0.25 * u_moltenDiffuseStrength, -vec4(hC.y * 0.25), vec4(hW.y, hE.y, hN.y, hS.y) * 0.25 );
			moltenHeight += (clampedDiffs.x + clampedDiffs.y + clampedDiffs.z + clampedDiffs.w) * DT;
			moltenHeight = max(0.0, moltenHeight);
		}
		
		float volumeC = hC.y;
		vec4 volumeN = vec4( hN.y, hS.y, hE.y, hW.y );

		// Sample from flux maps
		vec4 fluxC, fluxN; getFlux( s_moltenFluxData, fluxC, fluxN);
		
		// Sum how much volume is being exchanged, and their proportions
		float totalExported, totalImported, exportedProp;
		vec4 importedProp;
		getVolumeTransportVars( 
			volumeC, volumeN, fluxC, fluxN,								// Input
			totalExported, totalImported, exportedProp, importedProp	// Output
		);

		// Update volume
		moltenHeight += totalImported;
		moltenHeight -= totalExported;
		
		// Advect molten properties (heat and 'moltenScalar') using proportions calculated above
		float advectSpeed = smoothstep( 0.0, 0.001, volumeC ) * u_advectSpeed;
		vec4 advectedMiscData = advect( s_miscData, exportedProp, importedProp, advectSpeed );
		miscC.xy = advectedMiscData.xy;
	}

	//////////////////////////////
	// Water
	//////////////////////////////
	{
		float volumeC = hC.w;
		vec4 volumeN = vec4( hN.w, hS.w, hE.w, hW.w );

		// Sample from flux maps
		vec4 fluxC, fluxN; getFlux( s_waterFluxData, fluxC, fluxN);

		// Sum how much volume is being exchanged, and their proportions
		float totalExported, totalImported, exportedProp;
		vec4 importedProp;
		getVolumeTransportVars( 
			volumeC, volumeN, fluxC, fluxN,								// Input
			totalExported, totalImported, exportedProp, importedProp	// Output
		);

		// Update volume
		waterHeight += totalImported;
		waterHeight -= totalExported;


		{
			// Sum how much dirt volume is being exchanged
			float volumeC = hC.z;
			vec4 volumeN = vec4( hN.z, hS.z, hE.z, hW.z );

			float totalExported, totalImported, exportedProp;
			vec4 importedProp;
			getVolumeTransportVars( 
				volumeC, volumeN, fluxC * u_dirtTransportSpeed, fluxN * u_dirtTransportSpeed,	// Input
				totalExported, totalImported, exportedProp, importedProp						// Output
			);

			// Update volume
			dirtHeight += totalImported;
			dirtHeight -= totalExported;

			miscC.z = totalImported;
		}

		// Directly advect dirt between cells
		// Place transported amount into 'dissolvedDirt' field
		/*
		{
			float advectSpeed = smoothstep( 0.0, 0.001, volumeC ) * u_dirtTransportSpeed;
			vec4 advectedVolumes = advect( s_heightData, exportedProp, importedProp, advectSpeed );

			miscC.z = exportedProp * dirtHeight;

			dirtHeight = advectedVolumes.z;
		}
		*/

		// Advect water properties (dissolved dirt) using proportions calculated above
		
		//vec4 advectedMiscData = advect( s_miscData, exportedProp, importedProp, advectSpeed );
		//miscC.z = advectedMiscData.z;
	}
	
	out_heightData = max( vec4(0.0), vec4( solidHeight, moltenHeight, dirtHeight, waterHeight ) );
	out_miscData = miscC;
}