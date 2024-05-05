cbuffer CBuf2 : register(b0, space0)
{
    matrix transform;
    matrix normalmatrix;
};
cbuffer FrameCB : register(b0,space1)
{
    float4x4 View;
    float4x4 InvView;
    float4x4 Proj;
    float4x4 InvProj;
    float4x4 ViewProj;
    float4x4 InvViewProj;
    float4 EyePosW;
    float2 screenSize;
    float2 InvScreenSize;
    float NearZ;
    float FarZ;
    float TotalTime;
    float DeltaTime;
    float scale;
    float bias;
    float3 numClusters;
}
float4 main( float4 pos : POSITION ) : SV_Position
{
    return mul(mul(pos, transform), ViewProj);
}