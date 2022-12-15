Texture2D my_texture;
SamplerState my_sampler;

float4 main(float4 color : COLOR, float2 uv : UV) : SV_TARGET
{
    return my_texture.Sample(my_sampler, uv);
}