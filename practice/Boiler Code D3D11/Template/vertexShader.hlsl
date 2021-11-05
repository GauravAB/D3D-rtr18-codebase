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
	float3 tNorm: NORMAL0;
	float3 lightDirection: NORMAL1;
	float3 viewVector: NORMAL2;
};

vertex_output main(float4 pos:POSITION, float4 norm : NORMAL)
{
	vertex_output output;
	float4 eyeCoordinates = mul(viewMatrix, mul(worldMatrix, pos));
	output.tNorm = mul((float3x3)mul(viewMatrix, worldMatrix), (float3)norm);
	output.lightDirection = (float3)(lightPos - eyeCoordinates);
	output.viewVector = (float3)(-eyeCoordinates);
	output.position = mul(projectionMatrix, eyeCoordinates);

	return(output);
}


