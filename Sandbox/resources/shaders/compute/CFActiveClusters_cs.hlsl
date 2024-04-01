//Dispatch thread ID corresponds to each pixel
struct inputStruct
{
    uint2 screenSize;
    float scale;//numSlices/log(far/near)
    float bias;//numSlices * log(near)/log(far/near)
    uint3 numClusters; //number of threads used to dispatch
    float _pad0;
};
ConstantBuffer<inputStruct> inputBuffer;
Texture2D<float> zprepass;
RWByteAddressBuffer clusterActive;//these are actually bools

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
    clusterActive.Store(index * 4, 1);
}