TextureCube my_texture : register(t2);
SamplerState my_sampler;

float4 main(float3 pos : POSITION, float r : ROUGHNESS) : SV_TARGET
{
    float3 envColor = my_texture.SampleLevel(my_sampler, -pos, 1.2).rgb;
    
    envColor = envColor / (envColor + float3(1.0, 1.0, 1.0));
    envColor = pow(envColor, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
  
    return float4(envColor, 1.0);
}
