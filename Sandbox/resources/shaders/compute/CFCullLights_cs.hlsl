struct LightGridEntry
{
    uint offset;
    uint shadow_offset;
    uint size;
    uint _pad0;
};
struct ClusterAABB
{
    float4 minPoint;
    float4 maxPoint;
};
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
    float3 dimensions;
    float bias;
    uint numRegularLights;
    uint numShadowLights;
    uint numRegularDirLights;
    uint numShadowDirLights;
};
ConstantBuffer<inputStruct> inputBuffer : register(b0, space1);

StructuredBuffer<ClusterAABB> clustersBuffer : register(t0, space0);
StructuredBuffer<uint> activeClusters : register(t1, space0);

StructuredBuffer<float4> lightList : register(t2, space0);

RWStructuredBuffer<uint> countBuffer         : register(u3, space0); //the count buffer is a cpu visible buffer, that is used for counting
RWStructuredBuffer<uint> lightIndexList      : register(u4, space0);
RWStructuredBuffer<LightGridEntry> lightGrid : register(u5, space0);

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


#define ThreadX 2
#define ThreadY 2
#define ThreadZ 2
bool testSphereAABB(Light light, uint tile)
{
    float radius = light.exData.z;
    float3 center = mul(inputBuffer.View, float4(light.position, 1)).xyz;
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
[numthreads(1,1,1)]
void main( uint3 DTid : SV_DispatchThreadID)
{
    /*
       go through all clusters, each thread group takes one cluster
       
    */
    uint indexFlat = DTid.x + (DTid.y * inputBuffer.dimensions.x) + (DTid.z * inputBuffer.dimensions.x * inputBuffer.dimensions.y);
    if (indexFlat >= countBuffer[0])
    {
        return;
    }
    uint clusterIndex = activeClusters[indexFlat];
    uint RegularLightCount = inputBuffer.numRegularLights - inputBuffer.numRegularDirLights;
    uint ShadowLightCount = inputBuffer.numShadowLights - inputBuffer.numShadowDirLights;
    
    uint numVisibleLights = 0;
    uint numVisibleRegularLights = 0;
    uint visibleLights[128];
    
    for (uint i = 0; i < RegularLightCount; i++)
    {
        uint lightIndex = (i + inputBuffer.numRegularDirLights) * RegularLightStepSize;
        Light light = RegularLight(lightIndex);
        //point light
        if(light.type == 1)
        {
            //is the light visible
            if (testSphereAABB(light, clusterIndex))
            {
                visibleLights[numVisibleLights++] = lightIndex;//record the light index raw and not as a count
            }
        }//spot light
        else
        {
            visibleLights[numVisibleLights++] = lightIndex;
        }

    }
    numVisibleRegularLights = numVisibleLights;
    for (uint shd_i = 0; shd_i < ShadowLightCount; shd_i++)
    {
        uint lightIndex = (shd_i + inputBuffer.numShadowDirLights) * ShadowLightStepSize;
        lightIndex += RegularLightCount * RegularLightStepSize;
        Light light = RegularLight(lightIndex);
        if(light.type == 1)
        {
            if (testSphereAABB(light, clusterIndex))
            {
                visibleLights[numVisibleLights++] = lightIndex;
            }

        }
        else
        {
            visibleLights[numVisibleLights++] = lightIndex;
        }
    }
    uint offset;
    InterlockedAdd(countBuffer[1], numVisibleLights, offset);
    for (uint write_i = 0; write_i < numVisibleLights; write_i++)
    {
        lightIndexList[write_i + offset] = visibleLights[write_i];
    }
    printf("%u %u %u %u", clusterIndex, offset, numVisibleRegularLights + offset, numVisibleLights);
    lightGrid[clusterIndex].offset = offset;
    lightGrid[clusterIndex].shadow_offset = numVisibleRegularLights + offset;
    lightGrid[clusterIndex].size = numVisibleLights;
    
    
}