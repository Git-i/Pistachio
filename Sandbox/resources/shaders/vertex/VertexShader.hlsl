struct VS_OUT
{
    float3 worldpos : WORLD_POSITION;
	float3 normal : FRAGMENT_NORMAL;
    float2 UV : UV;
    float viewSpaceDepth : VS_DEPTH;
	float4 position : SV_POSITION;
};

cbuffer FrameCB : register(b0, space1)
{
    float4x4 View;
    float4x4 InvView;
    float4x4 Proj;
    float4x4 InvProj;
    float4x4 ViewProj;
    float4x4 InvViewProj;
    float4 EyePosW;
    float2 RenderTargetSize;
    float2 InvRenderTargetSize;
    float NearZ;
    float FarZ;
    float TotalTime;
    float DeltaTime;
    float4 numlights;
};
cbuffer ModelCB : register(b0, space0)
{
    matrix transform;
    matrix normalmatrix;
};

VS_OUT main(float3 pos : POSITION, float3 normal : NORMAL,float2 UV : UV)
{
	VS_OUT vso;
    vso.worldpos = mul(float4(pos, 1.0f), transform).xyz;
    vso.UV = UV;
    vso.normal = normalize(mul(normal, (float3x3) normalmatrix));
    vso.position = mul(mul(float4(pos, 1.0f), transform), ViewProj);
    float4 fragPosViewSpace = mul(float4(vso.worldpos, 1.0), View);
    vso.viewSpaceDepth = abs(fragPosViewSpace.z);

    return vso;
}