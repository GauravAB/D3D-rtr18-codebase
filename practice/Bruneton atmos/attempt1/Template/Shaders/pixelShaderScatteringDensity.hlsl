typedef float scalar;
typedef float3 scalar3;

#include "../atmosphere_definitions.h"
#include "atmosphere_model.hlsli"
#include "atmosphere_functions.hlsli"


//AlARM!!!! check this for XNA lib impl
#pragma pack_matrix(row_major)

cbuffer precompute_cb : register(b0)
{
	float3x3 luminance_from_radiance : packoffset(c0);
	int scattering_order : packoffset(c4.x);
}

struct PsInput
{
	float4 pos_ss : SV_POSITION;
	float2 uv	  : TEXCOORD0;
	uint slice_index : SV_RENDERTARGETARRAYINDEX;
};


struct PsOutput
{
	float3 scattering_density	:SV_TARGET0;
};

Texture2D transmittance_texture                 : register(t0);
Texture3D single_rayleigh_scattering_texture    : register(t1);
Texture3D single_mie_scattering_texture         : register(t2);
Texture3D multiple_scattering_texture           : register(t3);
Texture2D delta_irradiance_texture              : register(t4);

SamplerState atmosphere_sampler		:register(s0);


PsOutput main(PsInput input)
{
	PsOutput output = (PsOutput)0;

	float2 coord_ss = input.pos_ss.xy;

	//scattering density
	output.scattering_density = ComputeScatteringDensityTexture(atmosphere, transmittance_texture, single_rayleigh_scattering_texture, single_mie_scattering_texture,
		multiple_scattering_texture, delta_irradiance_texture, atmosphere_sampler, float3(coord_ss, input.slice_index + 0.5), scattering_order);

	return output;
}

