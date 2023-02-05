struct VS_OUT
{
    float3 worldpos : WORLD_POSITION;
	float3 normal : FRAGMENT_NORMAL;
    float2 UV : UV;
    float3 viewPos : V_POSITION;
    float numlights : NUM_LIGHTS;
    float4 lightSpacePos : LIGHT_SPACE_POS;
	float4 position : SV_POSITION;
};

cbuffer CBuf : register(b0)
{
	matrix viewProjection;
    float4 viewPos;
};
cbuffer CBuf2 : register(b1)
{
    matrix transform;
    matrix normalmatrix;
};
cbuffer shadowCbuf : register(b2)
{
    matrix lightSpaceMatrix[128];
    float4 numlights;
}
VS_OUT main(float3 pos : POSITION, float3 normal : NORMAL,float2 UV : UV)
{
	VS_OUT vso;
    vso.worldpos = mul(float4(pos, 1.0f), transform);
    vso.UV = UV;
    vso.normal = normalize(mul(normal, (float3x3) normalmatrix));
    vso.viewPos = viewPos;
    vso.position = mul(mul(float4(pos, 1.0f), transform), viewProjection);
    vso.numlights = numlights.x;
    vso.lightSpacePos = mul(float4(vso.worldpos, 1.f), lightSpaceMatrix[0]);
    return vso;
    }