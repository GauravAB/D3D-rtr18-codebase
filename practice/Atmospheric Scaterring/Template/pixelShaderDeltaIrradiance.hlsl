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
};


struct PsOutput
{
	float3 delta_irradiance;	:SV_TARGET0;
	float3 irradiance;			:SV_TARGET1;
};

Texture2D transmittance_texture		:register(t0);

SamplerState atmosphere_sampler		:register(s0);


PsOutput main(PsInput input)
{
	psOutput output = (PsOutput)0;

	float2 coord_ss = input.pos_ss.xy;

	//Irradiance
	output.delta_irradiance = ComputeDirectIrradianceTexture(atmosphere, transmittance_texture, atmosphere_sampler, coord_ss);
	output.irradiance = (float3)0.0;

	return output;
}











