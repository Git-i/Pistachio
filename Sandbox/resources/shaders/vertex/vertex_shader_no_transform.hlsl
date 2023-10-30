struct VS_OUT
{
    float2 uv : UV;
    float3 camPos : CAM_POS;
    int numlights : NUM_LIGHTS;
    float4 pos : SV_Position;
};
cbuffer CBuf : register(b0)
{
    matrix viewProjection;
    matrix view;
    float4 viewPos;
};
cbuffer shadowCbuf : register(b2)
{
    matrix lightSpaceMatrix[16];
    float4 numlights;
}
VS_OUT main(float3 pos : POSITION, float3 normal : NORMAL, float2 UV : UV)
{
    VS_OUT vso;
    vso.pos = float4(pos, 1.f);
    vso.camPos = viewPos;
    vso.numlights = numlights;
    vso.uv = UV;
	return vso;
}