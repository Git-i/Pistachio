
struct inputStruct
{
    uint3 csDimensions;
};
ConstantBuffer<inputStruct> inputBuffer  : register(b0, space1);
StructuredBuffer<uint> IsclusterActive : register(t0, space0);
RWStructuredBuffer<uint> counterBuffer   : register(u1, space0);
RWStructuredBuffer<uint> activeClusters  : register(u2, space0);
[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint index = DTid.x + (DTid.y * inputBuffer.csDimensions.x) + (DTid.z * inputBuffer.csDimensions.x * inputBuffer.csDimensions.y);
    if (IsclusterActive[0] == 1)
    {
        uint counterVal;
        InterlockedAdd(counterBuffer[0], 1, counterVal);
        activeClusters[counterVal] = index;
    }
}