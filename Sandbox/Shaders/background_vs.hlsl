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
struct VS_OUT
{
    float3 pos : POSITION;
    float4 position : SV_POSITION;
    float roughness : ROUGHNESS;
};

VS_OUT main( float3 pos : POSITION )
{
    VS_OUT vso;
    vso.pos = pos;
    vso.position = mul(float4(pos, 1.0f), viewProjection).xyww;
    vso.roughness = roughness;
    return vso;
}
