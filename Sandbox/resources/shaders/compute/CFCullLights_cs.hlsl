struct LightGridEntry
{
    uint offset;
    uint shadow_offset;
    uint size;
};
struct ClusterAABB
{
    float4 minPoint;
    float4 maxPoint;
};
struct inputStruct
{
    uint3 dimensions;
    uint numShadowDirLights;
    uint numRegularDirLights;
    uint numRegularLights;
    uint numTotalLights;
    float4x4 viewMatrix;
};
StructuredBuffer<ClusterAABB> clustersBuffer;
StructuredBuffer<uint> activeClusters;
ConstantBuffer<inputStruct> inputBuffer;

StructuredBuffer<float4> lightList;

RWStructuredBuffer<uint> countBuffer; //the count buffer is a cpu visible buffer, that is used for counting
RWStructuredBuffer<uint> lightIndexList;
RWStructuredBuffer<LightGridEntry> lightGrid;

static const uint RegularLightStepSize = 64 / 16;
static const uint ShadowLightStepSize = 336 / 16;

struct Light
{
    float3 position; // for directional lights this is direction and type
    int type;
    float4 colorxintensity;
    float4 exData;
    float4 rotation;
};
Light RegularLight(int startIndex)
{
    Light light;
    light.position = lightList[startIndex].xyz;
    light.type = asint(lightList[startIndex].w);
    light.colorxintensity = lightList[startIndex + 1];
    light.exData = lightList[startIndex + 2];
    light.rotation = lightList[startIndex + 3];
    return light;
}
//every shadow casting light starts with a light, and we dont the extra data to cull
struct grpLightData
{
    bool hasSpace;
    uint offset;
    uint numVisiblelights;
    uint numRegularLights;
    uint visiibleLights[127];
};
groupshared grpLightData grpData;

#define ThreadX 2
#define ThreadY 2
#define ThreadZ 2
bool testSphereAABB(Light light, uint tile)
{
    float radius = light.colorxintensity.w;
    float3 center = mul(inputBuffer.viewMatrix, float4(light.position, 1)).xyz;
    float sqDist = 0.0;
    ClusterAABB currentCell = clustersBuffer[tile];
    for (int i = 0; i < 3; i++)
    {
        float v = center[i];
        if (v < currentCell.minPoint[i])
        {
            sqDist += (currentCell.minPoint[i] - v) * (currentCell.minPoint[i] - v);
        }
        if (v > currentCell.maxPoint[i])
        {
            sqDist += (v - currentCell.maxPoint[i]) * (v - currentCell.maxPoint[i]);
        }
    }
    
    return sqDist <= (radius * radius);
}
static uint lightListOffset = 0;
[numthreads(ThreadX, ThreadY, ThreadZ)]
void main( uint3 DTid : SV_DispatchThreadID, uint3 groupIdx : SV_GroupID, uint localIndex : SV_GroupIndex)
{
    /*
       go through all clusters, each thread group takes one cluster
       
    */
    uint groupIndexFlat = groupIdx.x + (groupIdx.y * inputBuffer.dimensions.x) + (groupIdx.z * inputBuffer.dimensions.x * inputBuffer.dimensions.y);
    uint clusterIndex = activeClusters[groupIndexFlat];
    uint RegularLightCount = (inputBuffer.numRegularLights - inputBuffer.numRegularDirLights);
    uint ShadowLightCount = ((inputBuffer.numTotalLights - inputBuffer.numRegularLights) - inputBuffer.numShadowDirLights);
    
    uint groupSize = ThreadX * ThreadY * ThreadZ;
    uint numRegularBatches = (RegularLightCount * groupSize - 1) / groupSize;
    grpData.numVisiblelights = 0;
    for (uint batch = 0; batch < numRegularBatches; batch++)
    {
        uint lightIndex = (batch * groupSize + localIndex + inputBuffer.numRegularDirLights) * RegularLightStepSize;
        lightIndex = min(lightIndex, RegularLightCount * RegularLightStepSize);
        Light light = RegularLight(lightIndex);
        //point light
        if(light.type == 1)
        {
            //is the light visible
            if (testSphereAABB(light, clusterIndex))
            {
                uint index = 0;
                InterlockedAdd(grpData.numVisiblelights, 1, index);
                grpData.visiibleLights[index] = lightIndex;//record the light index raw and not as a count ??
            }
        }

    }
    GroupMemoryBarrierWithGroupSync(); //we insert a barrier, because we want regular lights to always be first
    grpData.numRegularLights = grpData.numVisiblelights;
    uint numShadowBatches = (ShadowLightCount * groupSize - 1) / groupSize;
    for (uint shd_batch = 0; shd_batch < numRegularBatches; shd_batch++)
    {
        uint lightIndex = (shd_batch * groupSize + localIndex + inputBuffer.numShadowDirLights) * ShadowLightStepSize;
        lightIndex += RegularLightCount * RegularLightStepSize;
        Light light = RegularLight(lightIndex);
        if(light.type == 1)
        {
            if (testSphereAABB(light, clusterIndex))
            {
                uint index = 0;
                InterlockedAdd(grpData.numVisiblelights, 1, index);
                grpData.visiibleLights[index] = lightIndex;
            }

        }
        
    }
    grpData.hasSpace = false;
    GroupMemoryBarrierWithGroupSync();
    //we basically allocate space and report our lights
    if(!grpData.hasSpace)
    {
        grpData.hasSpace = true;
        InterlockedAdd(countBuffer[0], grpData.numVisiblelights, grpData.offset);
        for (uint i = 0; i < grpData.numVisiblelights; i++)
        {
            lightIndexList[i + grpData.offset] = grpData.visiibleLights[i];
        }
    }
    //GroupMemoryBarrierWithGroupSync();  wait till all data has been put into the index list |?
    lightGrid[clusterIndex].offset = grpData.offset;
    lightGrid[clusterIndex].shadow_offset = grpData.offset = grpData.numRegularLights;
    lightGrid[clusterIndex].size = grpData.numVisiblelights;
    
}