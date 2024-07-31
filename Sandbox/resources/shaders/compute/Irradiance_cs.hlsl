Texture2DArray<float3> inputTexture;
Texture2DArray<float3> outputTexture;
SamplerState ss;
static float PI = 3.14159265359;
float3 GetSamplerVector(uint3 DTid)
{
    uint width, height, elem;
    outputTexture.GetDimensions(width, height, elem);
    float2 st = DTid.xy / float2(width, height);
    float2 uv = 2.0 * float2(st.x, 1.f - st.y) - 1.f.xx;
    float3 ret;
    switch (DTid.z)
    {
    case 0: ret = float3(1.0f, uv.y, -uv.x); break;
    case 1: ret = float3(-1.0f, uv.y, uv.x); break;
    case 2: ret = float3(uv.x, 1.0f, -uv.y); break;
    case 3: ret = float3(uv.x, -1.0f, uv.y); break;
    case 4: ret = float3(uv.x, uv.y, 1.0f); break;
    case 5: ret = float3(-uv.x, uv.y, -1.0f); break;
    }
    return ret;
}
[numthreads(1, 1, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
    float3 sampleVec = GetSamplerVector(DTid);
    //float3 normal = normalize(localPos);
    float3 irradiance = float3(0.0, 0.0, 0.0);

    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += inputTexture.Sample(ss, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
}