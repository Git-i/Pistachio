Texture2D my_texture : register(t0);
SamplerState my_sampler;

const float2 invAtan = float2(0.1591, 0.3183);
#define PI 3.14159265359
float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2((atan2(v.x, v.z) / (2 * PI) + 0.5), (asin(v.y) / PI + 0.5));
    return uv;
}

float4 main(float3 localPos : WORLD_POSITION) : SV_TARGET
{
    float2 uv = SampleSphericalMap(normalize(localPos));
    float3 color = my_texture.Sample(my_sampler, uv).rgb;
    return float4(color, 1.0f);
    
}
