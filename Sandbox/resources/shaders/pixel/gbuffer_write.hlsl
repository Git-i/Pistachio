Texture2D albedoTexture :    register(t0, space2);
Texture2D roughnessTexture : register(t1, space2);
Texture2D metallicTexture :  register(t2, space2);
Texture2D normalTexture :    register(t3, space2);
SamplerState my_sampler :    register(s4, space2);
cbuffer MaterialData : register(b5, space2)
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
    float4 pos : SV_Position;
};

struct PSOUTPUT
{
    float4 color : SV_Target0;
    float4 normal_rougness : SV_Target1;
    float4 position_metallic : SV_Target2;
    float4 shadow_info : SV_Target3;
};
PSOUTPUT main(PSINTPUT input)
{
    PSOUTPUT pso;
    pso.color = albedoMultiplier * albedoTexture.Sample(my_sampler, input.uv);
    pso.normal_rougness = float4(input.Normal, roughnessMultiplier * roughnessTexture.Sample(my_sampler, input.uv).x);
    pso.position_metallic = float4(input.WorldPos, metallicMultiplier * metallicTexture.Sample(my_sampler, input.uv).x);

    return pso;
}