//Dispatch thread ID corresponds to each pixel
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
    float3 numClusters;
    float bias;
};
ConstantBuffer<inputStruct> inputBuffer : register(b0, space1);
Texture2D<float> zprepass : register(t0, space0);
RWStructuredBuffer<uint> clusterActive : register(u1, space0); //these are actually bools

uint getSlice(float z)
{
    return (log10(z) * inputBuffer.scale) - inputBuffer.bias;
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    float z = zprepass.Load(DTid); // DTid z is always 0;
    uint slice = getSlice(z);
    float2 tileSize = float2(inputBuffer.screenSize) / float2(inputBuffer.numClusters.xy);
    float3 cluster = float3(float2(DTid.xy) / tileSize, slice);
    uint index = cluster.x + (cluster.y * inputBuffer.numClusters.x) + (cluster.z * inputBuffer.numClusters.x * inputBuffer.numClusters.y);
    clusterActive[0] =  1;
}