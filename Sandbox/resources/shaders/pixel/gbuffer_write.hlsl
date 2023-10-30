Texture2D albedoTexture : register(t3);
Texture2D roughnessTexture : register(t4);
Texture2D metallicTexture : register(t5);
Texture2D normalTexture : register(t6);
Texture2D shadowMap[4] : register(t9);

SamplerState my_sampler : register(s0);
SamplerState ShadowSampler : register(s2);
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
    float depthViewspace : SM_LAYER;
    int numlights : NUM_LIGHTS;
    float4 lightSpacePositions[16] : LS_POS;
};

struct PSOUTPUT
{
    float4 color : SV_Target0;
    float4 normal_rougness : SV_Target1;
    float4 position_metallic : SV_Target2;
    float4 shadow_info : SV_Target3;
    int id : SV_Target4;
};
float Shadow(float3 projCoords, int layer, int index)
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
    float closestDepth = shadowMap[index].Sample(ShadowSampler, float2(projCoords.x + x[layer], projCoords.y + y[layer])).r;
    
    float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
    return shadow;
}
PSOUTPUT main(PSINTPUT input)
{
    PSOUTPUT pso;
    pso.color = albedoMultiplier * albedoTexture.Sample(my_sampler, input.uv);
    pso.id = entityID;
    pso.normal_rougness = float4(input.Normal, roughnessMultiplier * roughnessTexture.Sample(my_sampler, input.uv).x);
    pso.position_metallic = float4(input.WorldPos, metallicMultiplier * metallicTexture.Sample(my_sampler, input.uv).x);
    int shadowMaplayer = 3;
    float cascadePlaneDistances[4] = { 10.f, 50.f, 70.f, 100.f };
    if (input.depthViewspace < cascadePlaneDistances[3])
    {
        shadowMaplayer = 3;
    }
    if (input.depthViewspace < cascadePlaneDistances[2])
    {
        shadowMaplayer = 2;
    }
    if (input.depthViewspace < cascadePlaneDistances[1])
    {
        shadowMaplayer = 1;
    }
    if (input.depthViewspace < cascadePlaneDistances[0])
    {
        shadowMaplayer = 0;
    }
    float3 lightSpacePos = input.lightSpacePositions[shadowMaplayer].xyz / input.lightSpacePositions[shadowMaplayer].w;
    pso.shadow_info.x = Shadow(lightSpacePos.xyz, shadowMaplayer, 0);
    
    lightSpacePos = input.lightSpacePositions[4+shadowMaplayer].xyz / input.lightSpacePositions[4+shadowMaplayer].w;
    pso.shadow_info.y = Shadow(lightSpacePos.xyz, shadowMaplayer,  1);
    
    lightSpacePos = input.lightSpacePositions[8+shadowMaplayer].xyz / input.lightSpacePositions[8+shadowMaplayer].w;
    pso.shadow_info.z = Shadow(lightSpacePos.xyz, shadowMaplayer, 2);
    
    lightSpacePos = input.lightSpacePositions[12+shadowMaplayer].xyz / input.lightSpacePositions[12+shadowMaplayer].w;
    pso.shadow_info.a = Shadow(lightSpacePos.xyz, shadowMaplayer, 3);
    return pso;
}