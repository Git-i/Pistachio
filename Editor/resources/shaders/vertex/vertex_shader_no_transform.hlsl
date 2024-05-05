struct VS_OUT
{
    float2 uv : UV;
    float3 camPos : CAM_POS;
    int numRegularlights : NUM_LIGHTS0;
    int numShadowlights : NUM_LIGHTS1;
    float4x4 view_mat : VIEW;
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
    int numRegularlights;
    int numShadowlights;
};
VS_OUT main(float3 pos : POSITION, float3 normal : NORMAL, float2 UV : UV)
{
    VS_OUT vso;
    vso.pos = float4(pos, 1.f);
    vso.camPos = EyePosW;
    vso.numRegularlights = numRegularlights;
    vso.numShadowlights = numShadowlights;
    vso.uv = UV;
    vso.view_mat = View;
	return vso;
}