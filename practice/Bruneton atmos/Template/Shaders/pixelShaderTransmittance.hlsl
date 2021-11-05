typedef float scalar;
typedef float3 scalar3;

#include "../atmosphere_definitions.h"
#include "atmosphere_model.hlsli"
#include "atmosphere_functions.hlsli"


#pragma pack_matrix(row_major)
cbuffer precompute_cb
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
	float4 transmittance	:SV_TARGET0;
};

Texture2D transmittance_texture;
SamplerState atmosphere_sampler;


PsOutput main(PsInput input)
{
	PsOutput output = (PsOutput)0;

	float2 coord_ss = input.pos_ss.xy;

	//transmittance              
	output.transmittance = float4(ComputeTransmittanceToTopAtmosphereBoundaryTexture(atmosphere, coord_ss), 1.0);
	
	return output;
}










