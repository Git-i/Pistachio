cbuffer CBuf
{
    matrix viewProjection;
    matrix transform;
};
struct VS_OUT
{
    float3 pos : POSITION;
    float4 position : SV_POSITION;
};

VS_OUT main( float3 pos : POSITION )
{
    VS_OUT vso;
    vso.pos = pos;
    vso.position = mul(float4(pos, 1.0f), viewProjection).xyww;
    return vso;
}
