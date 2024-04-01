
struct inputStruct
{
    uint3 csDimensions;
};
RWByteAddressBuffer IsclusterActive; //the last element of this struct is a uint counter
ConstantBuffer<inputStruct> inputBuffer;
RWStructuredBuffer<uint> activeClusters;
[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint index = DTid.x + (DTid.y * inputBuffer.csDimensions.x) + (DTid.z * inputBuffer.csDimensions.x * inputBuffer.csDimensions.y);
    uint counterIndex = inputBuffer.csDimensions.x * inputBuffer.csDimensions.y * inputBuffer.csDimensions.z;
    if (IsclusterActive.Load(index * 4))
    {
        uint counterVal;
        IsclusterActive.InterlockedAdd(counterIndex * 4, 1, counterVal);
        activeClusters[counterVal] = index;
    }
}