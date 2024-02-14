TextureCube my_texture : register(t0, space1);
SamplerState my_sampler : register(s1, space1);

float4 main(float3 pos : POSITION) : SV_TARGET
{
    float3 envColor = my_texture.Sample(my_sampler, pos).rgb;
    
    envColor = envColor / (envColor + float3(1.0, 1.0, 1.0));
    envColor = pow(envColor, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
  
    return float4(envColor, 1.0);
}
