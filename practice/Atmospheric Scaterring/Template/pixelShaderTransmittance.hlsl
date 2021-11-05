typedef float scalar;
typedef float3 scaler3;

#
#include "atmosphere_definations.h"
#include "atmosphere_model.hlsli"
#include "atmosphere_functions.hlsli"


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
	float4 transmittance;
};

Texture2D transmittance_texture		:register(t0);

SamplerState atmosphere_sampler		:register(s0);


PsOutput main(PsInput input)
{
	psOutput output = (PsOutput)0;

	float2 coord_ss = input.pos_ss.xy;

	//transmittance
	output.transmittance = float4(ComputeTransmittanceFromTopAtmosphereBoundryTexture(atomosphere, coord_ss), 1.0);
	
	return output;
}










