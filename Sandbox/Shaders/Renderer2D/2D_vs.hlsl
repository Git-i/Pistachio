cbuffer cb
{
    float4x4 viewproj;
};

struct VS_OUT
{
    float2 uv : UV;
    float4 color : COLOR;
    float texindex : TEXINDEX;
    int entityID : ID;
    float4 pos : SV_POSITION;
};
VS_OUT main( float3 pos : POSITION, float2 uv : UV, float4 color : COLOR, float index : TEXINDEX, int entityID : ID )
{
    VS_OUT vso;
    vso.pos = mul(float4(pos, 1.0), viewproj);
    vso.uv = uv;
    vso.color = color;
    vso.texindex = index;
    vso.entityID = entityID;
    return vso;
} 