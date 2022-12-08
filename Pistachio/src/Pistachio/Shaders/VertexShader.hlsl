struct VS_OUT
{
	float4 color : COLOR;
	float4 position : SV_POSITION;
};

VS_OUT main(float2 pos : POSITION, float4 color : COLOR)
{
	VS_OUT vso;
	vso.color = color;
	vso.position = float4(pos, 0.0f, 1.0f);
	return vso;
}