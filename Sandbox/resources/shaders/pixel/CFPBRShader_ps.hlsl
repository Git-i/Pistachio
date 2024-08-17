//The input structure should be common to all ShaderAssets
//set 0 is for the frame CB
cbuffer FrameCB : register(b0, space1)
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
    uint numRegularLights;
    uint numShadowLights;
    uint numRegularDirLights;
    uint numShadowDirLights;
};
//set 0 is vertex shader specific
//set 2 is for renderer specific stuff
struct LightGridEntry
{
    uint offset;
    uint shadow_offset;
    uint size;
    uint _pad0;
};
Texture2D<float2> BRDFLUT                : register(t0, space2);
TextureCube irradianceMap        : register(t1, space2);
TextureCube prefilterMap         : register(t2, space2);
Texture2D shadowMap              : register(t3, space2);

StructuredBuffer<LightGridEntry> lightGrid : register(t4, space2);
StructuredBuffer<float4> lights            : register(t5, space2);
StructuredBuffer<uint> lightIndices        : register(t6, space2);

SamplerState brdfSampler             : register(s7, space2);
SamplerState textureSampler          : register(s8, space2);
SamplerComparisonState ShadowSampler : register(s9, space2);

//set 3 is for material specific stuff
Texture2D diffuseTex   : register(t0, space3);
Texture2D mettalicTex  : register(t1, space3);
Texture2D roughnessTex : register(t2, space3);
Texture2D normalTex    : register(t3, space3);
//set 4 is because its a dynamic descriptor
cbuffer materialBuffer : register(b0, space4)
{
    float diffuseFac;
    float metallicFac;
    float roughnessFac;
};

struct PSINTPUT
{
    float3 WorldPos : WORLD_POSITION;
    float3 Normal : FRAGMENT_NORMAL;
    float2 uv : UV;
    float depthViewSpace : VS_DEPTH;
    float4 pos : SV_Position;
};
#define PI 3.14159265359
struct Light
{
    float3 position; // for directional lights this is direction and type
    int type;
    float4 colorxintensity;
    float4 exData;
    float4 rotation;
    
};

struct ShadowCastingLight
{
    Light light;
    float4x4 projection[4]; // used for frustum culling
    uint2 shadowMapOffset;
    uint2 shadowMapSize;
};
static const uint RegularLightStepSize = 64 / 16;
static const uint ShadowLightStepSize = 336 / 16;
Light RegularLight(int startIndex)
{
    Light light;
    light.position = lights[startIndex].xyz;
    light.type = asint(lights[startIndex].w);
    light.colorxintensity = lights[startIndex + 1];
    light.exData = lights[startIndex + 2];
    light.rotation = lights[startIndex + 3];
    return light;
}
//since matrices are colunm major
ShadowCastingLight ShadowLight(int startIndex)
{
    ShadowCastingLight light;
    light.light = RegularLight(startIndex);
    light.projection[0]._11_21_31_41 = lights[startIndex + 4];
    light.projection[0]._12_22_32_42 = lights[startIndex + 5];
    light.projection[0]._13_23_33_43 = lights[startIndex + 6];
    light.projection[0]._14_24_34_44 = lights[startIndex + 7];
    light.projection[1]._11_21_31_41 = lights[startIndex + 8];
    light.projection[1]._12_22_32_42 = lights[startIndex + 9];
    light.projection[1]._13_23_33_43 = lights[startIndex + 10];
    light.projection[1]._14_24_34_44 = lights[startIndex + 11];
    light.projection[2]._11_21_31_41 = lights[startIndex + 12];
    light.projection[2]._12_22_32_42 = lights[startIndex + 13];
    light.projection[2]._13_23_33_43 = lights[startIndex + 14];
    light.projection[2]._14_24_34_44 = lights[startIndex + 15];
    light.projection[3]._11_21_31_41 = lights[startIndex + 16];
    light.projection[3]._12_22_32_42 = lights[startIndex + 17];
    light.projection[3]._13_23_33_43 = lights[startIndex + 18];
    light.projection[3]._14_24_34_44 = lights[startIndex + 19];
    light.shadowMapOffset = asint(lights[startIndex + 20].xy);
    light.shadowMapSize = asint(lights[startIndex + 20].zw);
    return light;
}
float DistributionGGX(float3 N, float3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float3 N, float3 V, float3 L, float roughness);
float3 fresnelSchlick(float cosTheta, float3 F0);
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness);
float DirShadow(float3 projCoords, int layer, uint2 Offset, uint2 Size, float2 shadowMapSize);
float SpotShadow(float3 projCoords, uint2 Offset, uint2 Size, float2 shadowMapSize);
float Window(float distance, float max_distance);
float3 PBR(float3 N, float3 L, float3 V, Light light, float roughness, float attenuation, float3 diffuse, float3 F0, float metallic);
uint getSlice(float z, float scale, float bias)
{
    return (log10(z) * scale) - bias;
}
float4 main(PSINTPUT input) : SV_TARGET
{
    float4 diffuse = diffuseFac * diffuseTex.Sample(textureSampler, input.uv);
    float3 normal = input.Normal; //todo normal mapping
    float metallic = metallicFac * mettalicTex.Sample(textureSampler, input.uv).r;
    float roughness = roughnessFac * roughnessTex.Sample(textureSampler, input.uv).r;

    float2 tileSize = float2(screenSize) / float2(numClusters.xy);
    float2 shadowMapSize;
    shadowMap.GetDimensions(shadowMapSize[0], shadowMapSize[1]);
    //uint zslice = getSlice(input.depthViewSpace, scale, bias);
    float4 viewSpace = mul(float4(input.WorldPos, 1.0), View);
    float depthViewSpace = viewSpace.z;
    uint zslice = getSlice(depthViewSpace,scale,bias);
    
    uint3 cluster = uint3(uint2(input.pos.xy / tileSize), zslice);
    uint clusterIndex = cluster.x + (cluster.y * numClusters.x) + (cluster.z * numClusters.x * numClusters.y);
    
    LightGridEntry entry = lightGrid[clusterIndex];
    
    float3 N = normalize(normal);
    float3 WorldPos = input.WorldPos;
    float3 V = normalize(EyePosW.xyz - WorldPos);
    float3 R = reflect(-V, N);
    float3 F0 = 0.04.xxx;
    F0 = lerp(F0, diffuse.xyz, metallic);
    float3 Lo = float3(0.0, 0.0, 0.0);
    float4 final = float4(0, 0, 0, 1);
    for (int i = entry.offset; i < entry.shadow_offset; i++)
    {
        Light light = RegularLight(lightIndices[i]);
        // -------------Evaluate L and Attenuation-----------------//
        float3 L = normalize(light.position - WorldPos);
        
        float distance = length(light.position - WorldPos);
        float window = Window(distance, light.exData.z);
        float t = saturate((dot(light.rotation.xyz, -L) - light.exData.x) / (light.exData.y - light.exData.x));
        float attenuations[3] = { 1, window / (distance * distance), window * t * t / (distance * distance) };
        float attenuation = attenuations[light.type];
        //-----------------------------------------------------------//

        Lo += PBR(N, L, V, light, roughness, attenuation, diffuse.xyz, F0, metallic);
    }
    int shadowMaplayer = 3;

    for (int shd_i = entry.shadow_offset; shd_i < entry.size + entry.offset;shd_i++)
    {
        ShadowCastingLight light = ShadowLight(lightIndices[shd_i]);
        // -------------Calculate shadow and continue if object is occluded---------- //
        float shadow = 1.f;
        if (light.light.type == 2)
        {
            float4 lightSpacePos = mul(float4(WorldPos, 1.0), light.projection[0]);
            lightSpacePos = lightSpacePos / lightSpacePos.w;
            shadow = SpotShadow(lightSpacePos.xyz, light.shadowMapOffset, light.shadowMapSize, shadowMapSize);
        }
        
        //return shadow.xxxx;
        // -------------Evaluate L and Attenuation-----------------//
        float3 L = normalize(light.light.position - WorldPos);
        
        float distance = length(light.light.position - WorldPos);
        float window = Window(distance, light.light.exData.z);
        float t = saturate((dot(light.light.rotation.xyz, -L) - light.light.exData.x) / (light.light.exData.y - light.light.exData.x));
        float attenuations[3] = { 1, window / (distance * distance), window * t * t / (distance * distance) };
        float attenuation = attenuations[light.light.type];
        //-----------------------------------------------------------//

        Lo += PBR(N, L, V, light.light, roughness, attenuation, diffuse.xyz, F0, metallic) * shadow;
    }
    for(uint dir_i = 0; dir_i < numRegularDirLights * RegularLightStepSize; dir_i += RegularLightStepSize)
    {
        Light light = RegularLight(dir_i);
        float3 L = normalize(light.rotation.xyz);
        Lo += PBR(N, L, V, light, roughness, 1.0, diffuse.xyz, F0, metallic);
    }
    for(uint dir_shd_i = numRegularLights * RegularLightStepSize; 
        dir_shd_i <  (numShadowDirLights * ShadowLightStepSize) + (numRegularLights * RegularLightStepSize); dir_shd_i += ShadowLightStepSize)
    {
        ShadowCastingLight light = ShadowLight(dir_shd_i);
        float cascadePlaneDistances[4] = {light.light.position.xyz, zFar};
        if (depthViewSpace <= cascadePlaneDistances[3])
        {
            shadowMaplayer = 3;
        }
        if (depthViewSpace <= cascadePlaneDistances[2])
        {
            shadowMaplayer = 2;
        }
        if (depthViewSpace <= cascadePlaneDistances[1])
        {
            shadowMaplayer = 1;
        }
        if (depthViewSpace <= cascadePlaneDistances[0])
        {
            shadowMaplayer = 0;
        }
        float4 lightSpacePos = mul(float4(WorldPos, 1.0), light.projection[shadowMaplayer]);
        lightSpacePos = lightSpacePos / lightSpacePos.w;
        float shadow = DirShadow(lightSpacePos.xyz, shadowMaplayer, light.shadowMapOffset, light.shadowMapSize, shadowMapSize);
        float3 L = normalize(light.light.rotation.xyz);
        

        Lo += PBR(N, L, V, light.light, roughness, 1.0, diffuse.xyz, F0, metallic) * shadow;
    }
    float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    // -------------IBL Lighting-----------------------//
    float3 irradiance = irradianceMap.Sample(textureSampler, N).rgb;
    float3 indirectDiffuse = irradiance * diffuse.xyz;
    const float MAX_REFLECTION_LOD = 4;
    float3 prefilteredColor = prefilterMap.SampleLevel(textureSampler, -R, roughness * MAX_REFLECTION_LOD).rgb;
    float2 envBRDF = BRDFLUT.Sample(textureSampler, float2(max(dot(N, V), 0.0), roughness));
    
    float3 indirectSpecular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    // ------------------------------------------------//

    
    float3 ambient = (kD * indirectDiffuse + indirectSpecular) * 1;
   
    float3 color = ambient + Lo;
    // Tonemap
    color = color / (color + float3(1.0f, 1.0f, 1.0f));

    // Gamma Inverse Correct
    float invGamma = 1.0f / 2.2f;
    color = pow(color, float3(invGamma, invGamma, invGamma));
    //return colors[zslice % 6];
    return float4(color, diffuse.a);

}
float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float DirShadow(float3 projCoords, int layer, uint2 Offset, uint2 Size, float2 shadowMapSize)
{
    //ndc to uv space
    projCoords.x = (projCoords.x + 1) * 0.5;
    projCoords.y = (1 - projCoords.y) * 0.5;
    projCoords.z = 0 + projCoords.z * (1);
    float currentDepth = projCoords.z;
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
    float2 offset_uv = Offset / shadowMapSize;
    float2 size_uv = Size / (shadowMapSize * 2.f.xx);
    projCoords.xy *= size_uv;
    float4 x = { offset_uv.x, offset_uv.x + size_uv.x, offset_uv.x, offset_uv.x + size_uv.x };
    float4 y = { offset_uv.y, offset_uv.x, offset_uv.y + size_uv.y, offset_uv.y + size_uv.y };
    float2 uv = float2(projCoords.x + x[layer], projCoords.y + y[layer]);
    
    float closestDepth1 = shadowMap.SampleCmpLevelZero(ShadowSampler, uv, currentDepth).r;
    return closestDepth1;
}
float SpotShadow(float3 projCoords, uint2 Offset, uint2 Size, float2 shadowMapSize)
{
    projCoords.x = (projCoords.x + 1) * 0.5;
    projCoords.y = (1 - projCoords.y) * 0.5;
    projCoords.z = 0 + projCoords.z * (1);
    float currentDepth = projCoords.z;
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
    projCoords.xy *= Size / shadowMapSize;
    projCoords.xy += Offset / shadowMapSize;
    float closestDepth1 = shadowMap.SampleCmpLevelZero(ShadowSampler, projCoords.xy, currentDepth).r;
    return closestDepth1;
}
float Window(float distance, float max_distance)
{
    return pow(max((1 - pow((distance / max_distance), 4)), 0), 2);
}
float3 PBR(float3 N, float3 L, float3 V, Light light, float roughness, float attenuation, float3 diffuse,float3 F0, float metallic)
{
    float NdotL = dot(N, L);
    NdotL = max(NdotL, 0.0);
    float3 H = normalize(V + L);
    float3 radiance = (light.colorxintensity.xyz) * attenuation * light.colorxintensity.w;
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic;
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    float3 specular = numerator / max(denominator, 0.001);
    return (kD * diffuse / PI + specular) * radiance * NdotL;
}