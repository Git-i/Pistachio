Texture2D BRDFLUT : register(t0);
TextureCube irradianceMap : register(t1);
TextureCube prefilterMap : register(t2);
Texture2D color : register(t3);
Texture2D normal_t : register(t4);
Texture2D position_t : register(t5);
SamplerState sampler_s : register(s0);

float4 main(float2 uv : UV, float3 camPos : CAM_POS, int numlights : NUM_LIGHTS, float4 scr_pos : SV_Position) : SV_TARGET
{
    float4 in_color = color.Sample(sampler_s, uv);
    return in_color;
}