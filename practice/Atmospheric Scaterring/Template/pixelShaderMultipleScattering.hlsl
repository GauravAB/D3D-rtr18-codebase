typedef float scalar;
typedef float3 scaler3;

#include "atmosphere_definations.h"
#include "atmosphere_model.hlsli"
#include "atmosphere_functions.hlsli"


//AlARM!!!! check this for XNA lib impl
#pragma pack_matrix(row_major)

cbuffer precompute_cb				 : register(b0)
{
	float3x3 luminance_from_radiance : packoffset(c0);
	int scattering_order			 : packoffset(c4.x);
}

struct PsInput
{
	float4 pos_ss	 : SV_POSITION;
	float2 uv		 : TEXCOORD0;
	uint slice_index : SV_RENDERTARGETARRAYINDEX;

};


struct PsOutput
{
	float3 delta_multiple_scattering		:SV_TARGET0;
	float4 scattering						:SV_TARGET1;
};

Texture2D transmittance_texture                 : register(t0);
Texture3D scattering_density_texture            : register(t1);

SamplerState atmosphere_sampler					:register(s0);


PsOutput main(PsInput input)
{
	PsOutput output = (PsOutput)0;

	float2 coord_ss = input.pos_ss.xy;

	//multiple scattering
	float nu = 0.0;
	output.delta_multiple_scattering = ComputeMultipleScatteringTexture(atmosphere, transmittance_texture, scattering_density_texture, atmosphere_sampler, float3(coord_ss, input.slice_index + 0.5),
		nu);
	output.scattering = float4(mul(luminance_from_radiance, output.delta_multiple_scattering / RayleighPhaseFunction(nu)), 0.0);

	return output;
}


