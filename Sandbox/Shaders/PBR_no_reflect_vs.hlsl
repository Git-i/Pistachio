struct VS_OUT
{
    float3 worldpos : WORLD_POSITION;
    float3 normal : FRAGMENT_NORMAL;
    float2 UV : UV;
    float3 viewPos : V_POSITION;
    float3 albedo : ALBEDO;
    float metallic : METALLIC;
    float roughness : ROUGHNESS;
    float ao : AO;
    float4 position : SV_POSITION;
};

cbuffer CBuf
{
    matrix viewProjection;
    matrix transform;
    float4 viewPos;
    float4 albedo;
    float metallic;
    float roughness;
    float ao;
};

VS_OUT main(float3 pos : SEM0, float3 normal : SEM1, float2 UV : SEM2)
{
    VS_OUT vso;
    vso.worldpos = mul(float4(pos, 1.0f), transform);
    vso.UV = UV;
    vso.normal = mul(normal, (float3x3) transform);
    vso.viewPos = viewPos;
    vso.albedo = albedo;
    vso.metallic = metallic;
    vso.roughness = roughness;
    vso.ao = ao;
    vso.position = mul(mul(float4(pos, 1.0f), transform), viewProjection);
    return vso;
}