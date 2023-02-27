struct VS_OUT
{
    float3 worldpos : WORLD_POSITION;
	float3 normal : FRAGMENT_NORMAL;
    float2 UV : UV;
    float3 viewPos : V_POSITION;
    float depthViewSpace : SM_LAYER;
    int numlights : NUM_LIGHTS;
    float4 lightSpacePositions[16] : LS_POS;
	float4 position : SV_POSITION;
};

cbuffer CBuf : register(b0)
{
	matrix viewProjection;
    matrix view;
    float4 viewPos;
};
cbuffer CBuf2 : register(b1)
{
    matrix transform;
    matrix normalmatrix;
};

cbuffer shadowCbuf : register(b2)
{
    matrix lightSpaceMatrix[16];
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
    for (int i = 0; i < 4; i++)
    {
        vso.lightSpacePositions[i*4 + 0] = mul(float4(vso.worldpos, 1.f), lightSpaceMatrix[i*4 + 0]);
        vso.lightSpacePositions[i*4 + 1] = mul(float4(vso.worldpos, 1.f), lightSpaceMatrix[i*4 + 1]);
        vso.lightSpacePositions[i*4 + 2] = mul(float4(vso.worldpos, 1.f), lightSpaceMatrix[i*4 + 2]);
        vso.lightSpacePositions[i*4 + 3] = mul(float4(vso.worldpos, 1.f), lightSpaceMatrix[i*4 + 3]);
    }
    float4 fragPosViewSpace = mul(float4(vso.worldpos, 1.0), view);
    vso.depthViewSpace = abs(fragPosViewSpace.z);
    
    return vso;
}