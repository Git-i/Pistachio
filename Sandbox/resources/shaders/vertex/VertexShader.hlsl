struct VS_OUT
{
    float3 worldpos : WORLD_POSITION;
	float3 normal : FRAGMENT_NORMAL;
    float2 UV : UV;
    float3 viewPos : V_POSITION;
    float shadowMaplayer : SM_LAYER;
    int numlights : NUM_LIGHTS;
    float4 lightSpacePositions[4] : LS_POS;
    float shadowMapSize : SM_SIZE;
	float4 position : SV_POSITION;
};

cbuffer FrameCB : register(b0, space0)
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
};
cbuffer ModelCB : register(b0, space1)
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
    vso.viewPos = EyePosW.xyz;
    vso.position = mul(mul(float4(pos, 1.0f), transform), ViewProj);
    vso.numlights = numlights.x;
    vso.lightSpacePositions[0] = mul(float4(vso.worldpos, 1.f), lightSpaceMatrix[0]);
    vso.lightSpacePositions[1] = mul(float4(vso.worldpos, 1.f), lightSpaceMatrix[1]);
    vso.lightSpacePositions[2] = mul(float4(vso.worldpos, 1.f), lightSpaceMatrix[2]);
    vso.lightSpacePositions[3] = mul(float4(vso.worldpos, 1.f), lightSpaceMatrix[3]);
    float4 fragPosViewSpace = mul(float4(vso.worldpos, 1.0), View);
    float depthViewspace = abs(fragPosViewSpace.z);
    vso.shadowMaplayer = depthViewspace;
    vso.shadowMapSize = EyePosW.w;
    return vso;
}