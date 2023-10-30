struct GSOutput
{
	float4 pos : SV_POSITION;
    uint vp : SV_ViewportArrayIndex;
};
[maxvertexcount(12)]
void main(
	inout TriangleStream< GSOutput > output,
	triangle matrix lsm[3][4] : LSM,
	triangle float4 input[3] : SV_POSITION
)
{
    for (int f = 0; f < 4; ++f)
    {
        GSOutput element;
        element.vp = f;
        for (uint i = 0; i < 3; i++)
        {
            float4 pos = mul(input[i], lsm[0][f]);
            element.pos = pos;
            output.Append(element);
        }
        output.RestartStrip();
    }
}