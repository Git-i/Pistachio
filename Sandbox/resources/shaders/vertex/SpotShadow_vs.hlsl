cbuffer transform : register(b0, space0)
{
    matrix transform;
    matrix normalmatrix;
};
StructuredBuffer<uint> index : register(t1, space1); //todo: use a push constant
StructuredBuffer<float4> lights : register(t0, space1);
//since matrices are colunm major
struct Light
{
    float3 position;
    int type;
    float4 colorxintensity;
    float4 exData;
    float4 rotation;
    
};
struct ShadowCastingLight
{
    Light light;
    float4x4 projection[4];
    uint2 shadowMapOffset;
    uint2 shadowMapSize;
};
ShadowCastingLight ShadowLight(int startIndex)
{
    ShadowCastingLight light;
    //we only need the projection matrix
    light.projection[0]._11_21_31_41 = lights[startIndex + 4];
    light.projection[0]._12_22_32_42 = lights[startIndex + 5];
    light.projection[0]._13_23_33_43 = lights[startIndex + 6];
    light.projection[0]._14_24_34_44 = lights[startIndex + 7];
    light.shadowMapOffset = asint(lights[startIndex + 20].xy);
    light.shadowMapSize = asint(lights[startIndex + 20].zw);
    return light;
}
float4 main( float4 pos : POSITION ) : SV_POSITION
{
    ShadowCastingLight slight = ShadowLight(index[0]);
    return mul(mul(pos, transform), slight.projection[0]);
}