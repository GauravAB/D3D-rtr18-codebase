struct vertex_output
{
	float4 position: SV_POSITION;
	float3 tNorm: NORMAL0;
	float3 lightDirection:NORMAL1;
	float3 viewVector:NORMAL2;
};

cbuffer phongBuffer
{
	float4 la;
	float4 ld;
	float4 ls;
	float4 ka;
	float4 kd;
	float4 ks;
	float shininess;
	uint togglekey;
}

float4 main(float4 position:SV_POSITION, vertex_output input):SV_TARGET
{
	float3 normal = normalize(input.tNorm);
	float3 source = normalize(input.lightDirection);
	float4 ambient = ka * la;
	float4 diffuse = kd * ld * max(dot(normal, source), 0.0);
	float3 reflectionVector = reflect(-source, normal);
	float spec = pow(max(dot(reflectionVector, normalize(input.viewVector)), 0.0), shininess);
	float4 specular = ks * ls * spec;
	float4 color = float4(1.0, 1.0, 1.0, 1.0);

	if (togglekey == 0)
	{

	}
	else
	{
		color = ambient + diffuse + specular;
	}

	return (color);
}
