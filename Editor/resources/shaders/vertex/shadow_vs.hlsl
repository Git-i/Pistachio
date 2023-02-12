cbuffer CBuf2 : register(b1)
{
    matrix transform;
    matrix normalmatrix;
};
cbuffer shadowCbuf : register(b2)
{
    matrix lightSpaceMatrix[128];
    float4 numlights;
}
struct VS_OUT
{
    matrix lsm[4] : LSM;
    float4 pos : SV_Position;
};
VS_OUT main( float4 pos : POSITION ) 
{
    VS_OUT vso;
    vso.lsm[0] = lightSpaceMatrix[(uint)(numlights.x*4) + 0];
    vso.lsm[1] = lightSpaceMatrix[(uint)(numlights.x*4) + 1];
    vso.lsm[2] = lightSpaceMatrix[(uint)(numlights.x*4) + 2];
    vso.lsm[3] = lightSpaceMatrix[(uint)(numlights.x*4) + 3];
    vso.pos = mul(pos, transform);
    return vso;
}