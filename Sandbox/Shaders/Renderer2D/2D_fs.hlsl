Texture2D texture1[32] : register(t0);
SamplerState texsampler;

struct PS_OUT
{
    float4 color1 : SV_Target0;
    int entityID : SV_Target1;
};

PS_OUT main(float2 uv : UV, float4 color : COLOR, float index : TEXINDEX, int ID : ID)
{
    float4 retval;
    for (int i = 0; i < 32; i++)
    {
        if (i == (int)index)
        {
            retval = texture1[i].Sample(texsampler, uv) * color;
            break;
        }
    }
    if (retval.a <= 0.1f)
        discard;
    PS_OUT pso;
    pso.color1 = retval;
    pso.entityID = ID;
    return pso;
}
