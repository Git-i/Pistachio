
Texture2D albedoTexture : register(t3);
Texture2D roughnessTexture : register(t4);
Texture2D metallicTexture : register(t5);
Texture2D normalTexture : register(t6);
#ifdef SHADOW_MAP
Texture2D CascadedShadowMap : register(t9);
#endif
SamplerState my_sampler : register(s0);
SamplerComparisonState ShadowSampler : register(s2);
cbuffer MaterialData : register(b1)
{
    float4 albedoMultiplier;
    float metallicMultiplier;
    float roughnessMultiplier;
    int entityID;
}

struct PSINTPUT
{
    float3 WorldPos : WORLD_POSITION;
    float3 Normal : FRAGMENT_NORMAL;
    float2 uv : UV;
    float3 camPos : V_POSITION;
    float depthViewSpace : SM_LAYER;
    int numlights : NUM_LIGHTS;
    float shadowMapSize : SM_SIZE;
    float4 lightSpacePositions[4] : LS_POS;
    float4 pos : SV_Position;
};

struct PSOUTPUT
{
    float4 color : SV_Target0;
    float4 normal_rougness : SV_Target1;
    float4 position_metallic : SV_Target2;
    float4 shadow_info : SV_Target3;
};
#ifdef SHADOW_MAP
float Shadow(float3 projCoords, int layer)
{
    projCoords.x = (projCoords.x + 1) * 0.25;
    projCoords.y = (1 - projCoords.y) * 0.25;
    projCoords.z = 0 + projCoords.z * (1);
    float currentDepth = projCoords.z;
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
    float x[4] = { 0, 0.5, 0, 0.5 };
    float y[4] = { 0, 0, 0.5, 0.5 };
    float closestDepth1 = CascadedShadowMap.SampleCmpLevelZero(ShadowSampler, float2(projCoords.x + x[layer], projCoords.y + y[layer]), currentDepth).r;
    
    return 1.f-closestDepth1;
}
#endif
PSOUTPUT main(PSINTPUT input)
{
    PSOUTPUT pso;
    pso.color = albedoMultiplier * albedoTexture.Sample(my_sampler, input.uv);
    pso.normal_rougness = float4(input.Normal, roughnessMultiplier * roughnessTexture.Sample(my_sampler, input.uv).x);
    pso.position_metallic = float4(input.WorldPos, metallicMultiplier * metallicTexture.Sample(my_sampler, input.uv).x);
    #ifdef SHADOW_MAP
    float4 colors[4] = { float4(1.f, 0.3f, 0.3f, 1.f), float4(1.f, 1.f, 0.33f, 1.f), float4(0.3f, 0.2f, 1.f, 1.f), float4(0.f, 0.7f, 0.23f, 1.f) };
    int shadowMaplayer = 3;
    float cascadePlaneDistances[4] = { 30.f, 100.f, 500.f, 1000.f };
    if (input.depthViewSpace <= cascadePlaneDistances[3])
    {
        shadowMaplayer = 3;
    }
    else if (input.depthViewSpace <= cascadePlaneDistances[2])
    {
        shadowMaplayer = 2;
    }
    else if (input.depthViewSpace <= cascadePlaneDistances[1])
    {
        shadowMaplayer = 1;
    }
    else if (input.depthViewSpace <= cascadePlaneDistances[0])
    {
        shadowMaplayer = 0;
    }
    float3 lightSpacePos = input.lightSpacePositions[shadowMaplayer].xyz / input.lightSpacePositions[shadowMaplayer].w;
    pso.shadow_info.x = Shadow(lightSpacePos.xyz, shadowMaplayer);
    #endif
    return pso;
}