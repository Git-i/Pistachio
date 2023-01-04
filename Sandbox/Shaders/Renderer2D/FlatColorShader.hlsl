Texture2D my_texture : register(t4);
SamplerState texsampler;
cbuffer Cbuf
{
    float4 color;
};
float4 main(float2 pos : POSITION) : SV_TARGET
{
    float4 retval = my_texture.Sample(texsampler, float2(pos.x, -pos.y) * 0.5 + float2(0.5, 0.5)) * color;
    if (retval.a <= 0.1f)
        discard;
    return color;
}