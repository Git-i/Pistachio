cbuffer FrameCB : register(b0)
{
    float4x4 View;
    float4x4 InvView;
    float4x4 Proj;
    float4x4 InvProj;
    float4x4 ViewProj;
    float4x4 InvViewProj;
};
struct VS_OUT
{
    float3 pos : POSITION;
    float4 position : SV_POSITION;
};

VS_OUT main( float3 pos : POSITION )
{
    VS_OUT vso;
    vso.pos = pos;
    vso.position = mul(float4(pos, 1.0f), ViewProj).xyww;
    return vso;
}
