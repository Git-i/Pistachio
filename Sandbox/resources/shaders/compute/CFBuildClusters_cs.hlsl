//build clusters for clustered shading
//implementation is based on https://www.aortiz.me/2018/12/21/CG.html
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
struct ClusterAABB
{
    float4 minPoint;
    float4 maxPoint;
};
RWStructuredBuffer<ClusterAABB> clustersBuffer : register(u0, space0);
ConstantBuffer<inputStruct> inputBuffer : register(b0, space1);

float4 screen2view(float4 screen)
{
    //Convert to NDC
    float2 texCoord = screen.xy / float2(inputBuffer.screenSize.xy);

    //Convert to clipSpace
    // vec4 clip = vec4(vec2(texCoord.x, 1.0 - texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
    float4 clip = float4(texCoord.xy * 2.0 - 1.0, screen.z, screen.w);
    float4 view = mul(inputBuffer.InvProj, clip);
    
    view = view / view.w;
    
    return view;
}
float3 lineIntersectionToZPlane(float3 A, float3 B, float zDistance)
{
    //all clusters planes are aligned in the same z direction
    float3 normal = float3(0.0, 0.0, 1.0);
    //getting the line from the eye to the tile
    float3 ab = B - A;
    //Computing the intersection length for the line and the plane
    float t = (zDistance - dot(normal, A)) / dot(normal, ab);
    //Computing the actual xyz position of the point along the line
    float3 result = A + t * ab;
    return result;
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint index = DTid.x + (DTid.y * inputBuffer.csDimensions.x) + (DTid.z * inputBuffer.csDimensions.x * inputBuffer.csDimensions.y);
    float2 tileSize = float2(inputBuffer.screenSize) / float2(inputBuffer.csDimensions.xy);
    float4 clusterMin_SS = float4(tileSize * DTid.xy,1,1);
    float4 clusterMax_SS = float4(tileSize * (DTid.xy + 1.xx), 1, 1);
    float tileNear = inputBuffer.zNear * pow(inputBuffer.zFar / inputBuffer.zNear, float(DTid.z) / float(inputBuffer.csDimensions.z));
    float tileFar = inputBuffer.zNear * pow(inputBuffer.zFar / inputBuffer.zNear, float(DTid.z+1) / float(inputBuffer.csDimensions.z));
    float3 clusterMax_VS = screen2view(clusterMax_SS).xyz;
    float3 clusterMin_VS = screen2view(clusterMin_SS).xyz;
    float3 minPointNear = lineIntersectionToZPlane(0.xxx, clusterMin_VS, tileNear);
    float3 minPointFar = lineIntersectionToZPlane (0.xxx, clusterMin_VS, tileFar);
    float3 maxPointNear = lineIntersectionToZPlane(0.xxx, clusterMax_VS, tileNear);
    float3 maxPointFar = lineIntersectionToZPlane (0.xxx, clusterMax_VS, tileFar);
    
    float3 minPointAABB = min(min(minPointNear, minPointFar), min(maxPointNear, maxPointFar));
    float3 maxPointAABB = max(max(minPointNear, minPointFar), max(maxPointNear, maxPointFar));
    printf("index: %u", index);
    clustersBuffer[index].minPoint = float4(minPointAABB, 0.0);
    clustersBuffer[index].maxPoint = float4(maxPointAABB, 0.0);
}