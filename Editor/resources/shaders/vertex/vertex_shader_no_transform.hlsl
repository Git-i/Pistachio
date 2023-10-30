struct VS_OUT
{
    float2 uv : UV;
    float3 camPos : CAM_POS;
    int numlights : NUM_LIGHTS;
    float4 pos : SV_Position;
};
cbuffer FrameCB : register(b0)
{
    float4x4 View;
    float4x4 InvView;
    float4x4 Proj;
    float4x4 InvProj;
    float4x4 ViewProj;
    float4x4 InvViewProj;
    float3 EyePosW;
    float shadowMapSize;
    float2 RenderTargetSize;
    float2 InvRenderTargetSize;
    float NearZ;
    float FarZ;
    float TotalTime;
    float DeltaTime;
    matrix lightSpaceMatrix[16];
    float4 numlights;
};
VS_OUT main(float3 pos : POSITION, float3 normal : NORMAL, float2 UV : UV)
{
    VS_OUT vso;
    vso.pos = float4(pos, 1.f);
    vso.camPos = EyePosW;
    vso.numlights = numlights;
    vso.uv = UV;
	return vso;
}