struct VS_OUT
{
    float3 localPos : WORLD_POSITION;
    float4 position : SV_POSITION;
};

cbuffer CBuf
{
    matrix viewProjection;
    matrix transform;
};

VS_OUT main( float3 pos : POSITION )
{
    VS_OUT vso;
    vso.localPos = pos.xyz;
    vso.position = mul(float4(pos, 1.0), viewProjection);
    return vso;
}