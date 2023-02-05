struct VS_OUT
{
    float2 TexCoords : UV;
    float4 position : SV_POSITION;
};

VS_OUT main( float3 pos : POSITION, float3 normal:NORMAL,float2 UV : UV )
{
    VS_OUT vso;
    vso.TexCoords = float2(UV.y, -UV.x);
    vso.position = float4(-pos.x, pos.y, pos.z, 1.0);
	return vso;
}