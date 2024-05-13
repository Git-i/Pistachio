RWTexture2DArray<float3> outCubeMap;
Texture2D inTexture;
SamplerState ss;
#define PI 3.14159265359
float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2((atan2(v.x, v.z) / (2 * PI) + 0.5), (asin(v.y) / PI + 0.5));
    return uv;
}
float3 GetSamplerVector(uint3 DTid)
{
    uint width, height, elem;
    outCubeMap.GetDimensions(width, height, elem);
    float2 st = DTid.xy/float2(width, height);
    float2 uv = 2.0 * float2(st.x, 1.f - st.y) - 1.f.xx;
    float3 ret;
    switch(DTid.z)
    {
        case 0: ret = float3( 1.0f,  uv.y, -uv.x); break;
        case 1: ret = float3(-1.0f,  uv.y,  uv.x); break;
        case 2: ret = float3( uv.x,  1.0f, -uv.y); break;
        case 3: ret = float3( uv.x, -1.0f,  uv.y); break;
        case 4: ret = float3( uv.x,  uv.y,  1.0f); break;
        case 5: ret = float3(-uv.x,  uv.y, -1.0f); break;
    }
    return ret;
}
[numthreads(1,1,1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float3 transformed = GetSamplerVector(DTid);
    outCubeMap[DTid] = inTexture.SampleLevel(ss,SampleSphericalMap(transformed),0).xyz;
}