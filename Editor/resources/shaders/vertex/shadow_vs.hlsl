cbuffer CBuf2 : register(b1)
{
    matrix transform;
    matrix normalmatrix;
};
cbuffer shadowCbuf : register(b0)
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
    matrix lightSpaceMatrix[16];
    float4 numlights;
}
struct VS_OUT
{
    matrix lsm[4] : LSM;
    float4 pos : SV_Position;
};
VS_OUT main( float4 pos : POSITION ) 
{
    VS_OUT vso;
    vso.lsm[0] = lightSpaceMatrix[0];
    vso.lsm[1] = lightSpaceMatrix[0];
    vso.lsm[2] = lightSpaceMatrix[0];
    vso.lsm[3] = lightSpaceMatrix[0];
    vso.pos = mul(pos, transform);
    return vso;
}