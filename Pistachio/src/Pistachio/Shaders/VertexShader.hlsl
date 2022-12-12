struct VS_OUT
{
	float4 color : COLOR;
    float2 UV : UV;
	float4 position : SV_POSITION;
};

cbuffer CBuf
{
	matrix viewProjection;
	matrix transform;
    float time;
};

VS_OUT main(float3 pos : POSITION, float2 UV : UV)
{
	VS_OUT vso;
    vso.UV = UV;
    vso.color = float4(UV.xy, 0.0f, 1.0f);
    vso.position = mul(mul(float4(pos, 1.0f), transform), viewProjection);
    return vso;
    }