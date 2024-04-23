
struct inputStruct
{
    float4x4 View;
    float4x4 InvView;
    float4x4 Proj;
    float4x4 InvProj;
    float4x4 ViewProj;
    float4x4 InvViewProj;
    float2 screenSize;
    float2 InvScreenSize;
    float zNear;
    float zFar;
    float TotalTime;
    float DeltaTime;
    float3 EyePosW;
    float scale;
    float3 csDimensions;
    float bias;
};
ConstantBuffer<inputStruct> inputBuffer  : register(b0, space1);
StructuredBuffer<uint> IsclusterActive : register(t0, space0);
RWStructuredBuffer<uint> counterBuffer   : register(u1, space0);
RWStructuredBuffer<uint> activeClusters  : register(u2, space0);
[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint index = DTid.x + (DTid.y * inputBuffer.csDimensions.x) + (DTid.z * inputBuffer.csDimensions.x * inputBuffer.csDimensions.y);
    if (IsclusterActive[index] == 1)
    {
        uint counterVal;
        InterlockedAdd(counterBuffer[0], 1, counterVal);
        activeClusters[counterVal] = index;
    }
}