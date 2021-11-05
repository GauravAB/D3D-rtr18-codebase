
Texture2D myTexture2D;
SamplerState mySamplerState;

float4 main(float4 position:SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float4 fColor = myTexture2D.Sample(mySamplerState,float2(texcoord.x,texcoord.y));

	return float4(fColor);
}
