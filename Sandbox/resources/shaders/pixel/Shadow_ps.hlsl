float4 main(float3 worldpos: WORLD_POSITION) : SV_TARGET
{
	return float4(worldpos.zzz, 1.0f);
}