typedef float scalar;
typedef float3 scaler3;


#include "atmosphere_definations.h"
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
	float3 delta_irradiance		:SV_TARGET0;
	float3 irradiance			:SV_TARGET1;
};

Texture3D single_rayleigh_scattering_texture    : register(t1);
Texture3D single_mie_scattering_texture         : register(t2);
Texture3D multiple_scattering_texture           : register(t3);

SamplerState atmosphere_sampler		:register(s0);


PsOutput main(PsInput input)
{
	psOutput output = (PsOutput)0;

	float2 coord_ss = input.pos_ss.xy;

	//indirect irradiance
	output.delta_irradiance = ComputeIndirectIrradianceTexture(atmosphere, single_rayleigh_scattering_texture, single_mie_scattering_texture, multiple_scattering_texture,
		atmosphere_sampler, float3(coord_ss, input.slice_index + 0.5), scattering_order);
	output.irradiance = mul(luminance_from_radiance, output.delta_irradiance);

	return output;
}

