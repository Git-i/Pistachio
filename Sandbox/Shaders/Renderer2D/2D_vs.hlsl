cbuffer cb
{
    float4x4 viewproj;
};
cbuffer cb : register(b1)
{
    float4x4 transform;
};
struct VS_OUT
{
    float2 position : POSITION;
    float4 pos : SV_POSITION;
};
VS_OUT main( float4 pos : POSITION )
{
    VS_OUT vso;
    vso.pos = mul(mul(pos, transform), viewproj);
    vso.position = pos.xy;
    vso.pos = pos;
    return vso;
} 