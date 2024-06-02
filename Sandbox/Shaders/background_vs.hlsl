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
    matrix view = View;
    view._14_24_34 = 0.xxx;
    view._41_42_43 = 0.xxx;
    view._44 = 1.f;
    matrix vp = view * Proj;
    vso.position = mul(float4(pos, 1.0f), vp);
    vso.position = vso.position.xyww;
    return vso;
}
