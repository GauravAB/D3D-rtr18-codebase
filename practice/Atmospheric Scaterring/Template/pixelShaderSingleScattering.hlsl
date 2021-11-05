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
	float3 delta_rayleigh :SV_TARGET0;
	float3 delta_mie	  :SV_TARGET1;
	float4 scattering	  :SV_TARGET2;
	float3 single_mie	  :SV_TARGET3;
};

Texture2D transmittance_texture		:register(t0);

SamplerState atmosphere_sampler		:register(s0);


PsOutput main(PsInput input)
{
	psOutput output = (PsOutput)0;

	float2 coord_ss = input.pos_ss.xy;

	//single scattering
	ComputeSingleScatteringTexture(atmosphere, transmittance_texture, atmosphere_sampler, float3(coord_ss, input.slice + 0.5), output.delta_rayleigh, output.delta_mie);
	output.scattering = float4(mul(luminance_from_radiance, output.delta_rayleigh), mul(luminance_from_radiance, output.delta_mie).r);
	output.single_mie = mul(luminance_from_radiance, output.delta_mie);

	return output;
}

