cbuffer ConstantBuffer
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4 lightPos;
};

struct vertex_output
{
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
};

vertex_output main( float4 pos : POSITION , float2 texcoord:TEXCOORD)
{
	vertex_output output;
	float4x4 worldViewMatrix = mul(viewMatrix, worldMatrix);
	float4x4 worldViewProjectionMatrix = mul(projectionMatrix, worldViewMatrix);

	output.position = mul(worldViewProjectionMatrix, pos);
	output.texcoord = texcoord;

	return output;
}