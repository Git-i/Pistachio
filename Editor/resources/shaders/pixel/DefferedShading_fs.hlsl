Texture2D BRDFLUT : register(t0);
TextureCube irradianceMap : register(t1);
TextureCube prefilterMap : register(t2);


Texture2D color : register(t3);
Texture2D normal_roughness : register(t4);
Texture2D position_metallic : register(t5);
Texture2D shadow_t : register(t9);


SamplerState my_sampler : register(s0);
SamplerState Brdfsampler : register(s1);
SamplerComparisonState ShadowSampler : register(s2);


float DistributionGGX(float3 N, float3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float3 N, float3 V, float3 L, float roughness);
float3 fresnelSchlick(float cosTheta, float3 F0);
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness);
float DirShadow(float3 projCoords, int layer, uint2 Offset, uint2 Size);
float SpotShadow(float3 projCoords, uint2 Offset, uint2 Size);
float Window(float distance, float max_distance);
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
StructuredBuffer<float4> lights : register(t7);  //16 bytes per Element being the gcd of the size of normal and shadow light
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
    light.shadowMapSize =   asint(lights[startIndex + 20].zw);
    return light;
}
float4 main(float2 uv : UV, float3 camPos : CAM_POS, int numRegularlights : NUM_LIGHTS0, int numShadowlights : NUM_LIGHTS1, float4x4 view_mat : VIEW) : SV_Target
{
    float4 albedo = color.Sample(my_sampler, uv);
    float4 normal_rough = normal_roughness.Sample(my_sampler, uv);
    float4 position_metal = position_metallic.Sample(my_sampler, uv);
    float metallic = position_metal.w;
    float roughness = normal_rough.w;
    clip(albedo.a < 0.1f ? -1 : 1);
    float3 N = normalize(normal_rough.xyz);
    float3 WorldPos = position_metal.xyz;
    float3 V = normalize(camPos.xyz - WorldPos);
    float3 R = reflect(-V, N);
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo.xyz, metallic);
    float l;
    // reflectance equation
    float3 Lo = float3(0.0, 0.0, 0.0);
    // Lighting
    for (int i = 0; i < numRegularlights * RegularLightStepSize; i += RegularLightStepSize)
    {
        Light light = RegularLight(i);
        // -------------Evaluate L and Attenuation-----------------//
        float3 Ls[3] = { normalize(light.rotation.xyz), normalize(light.position - WorldPos), float3(0, 0, 0) };
        Ls[2] = Ls[1];
        float3 L = Ls[light.type];
        
        float distance = length(light.position - WorldPos);
        float window = Window(distance, light.exData.z);
        float t = saturate((dot(light.rotation.xyz, -L) - light.exData.x) / (light.exData.y - light.exData.x));
        float attenuations[3] = { 1, window / (distance * distance), window * t * t / (distance * distance) };
        float attenuation = attenuations[light.type];
        //-----------------------------------------------------------//
        float NdotL = dot(N, L);
        NdotL = max(NdotL, 0.0);
        float3 H = normalize(V + L);
        float3 radiance = (light.colorxintensity.xyz) * attenuation * light.colorxintensity.w;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - metallic;

        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        float3 specular = numerator / max(denominator, 0.001);

        Lo += (kD * albedo.xyz / PI + specular) * radiance * NdotL;
    }
    int shadowMaplayer = 3;
    float cascadePlaneDistances[4] = { 30.f, 100.f, 500.f, 1000.f };
    float depthViewSpace = abs(mul(float4(WorldPos, 1.0), view_mat).z);
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
    for (int j = numRegularlights * RegularLightStepSize; j < (numShadowlights * ShadowLightStepSize) + (numRegularlights * RegularLightStepSize); j += ShadowLightStepSize)
    {
        ShadowCastingLight light = ShadowLight(j);
        // -------------Calculate shadow and continue if object is occluded---------- //
        float4 lightSpacePos = mul(float4(WorldPos, 1.0), light.projection[shadowMaplayer]);
        lightSpacePos = lightSpacePos / lightSpacePos.w;
        //return (lightSpacePos);
        float shadow;
        if(light.light.type == 0)
            shadow = DirShadow(lightSpacePos.xyz, shadowMaplayer, light.shadowMapOffset, light.shadowMapSize);
        else if(light.light.type == 2)
            shadow = SpotShadow(lightSpacePos.xyz, light.shadowMapOffset, light.shadowMapSize);
        //return shadow.xxxx;
        // -------------Evaluate L and Attenuation-----------------//
        float3 Ls[3] = { normalize(light.light.rotation.xyz), normalize(light.light.position - WorldPos), float3(0, 0, 0) };
        Ls[2] = Ls[1];
        float3 L = Ls[light.light.type];
        
        float distance = length(light.light.position - WorldPos);
        float window = Window(distance, light.light.exData.z);
        float t = saturate((dot(light.light.rotation.xyz, -L) - light.light.exData.x) / (light.light.exData.y - light.light.exData.x));
        float attenuations[3] = { 1, window / (distance * distance), window * t * t / (distance * distance) };
        float attenuation = attenuations[light.light.type];
        //-----------------------------------------------------------//
        float NdotL = dot(N, L);
        NdotL = max(NdotL, 0.0);
        float3 H = normalize(V + L);
        float3 radiance = (light.light.colorxintensity.xyz) * attenuation * light.light.colorxintensity.w;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - metallic;

        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        float3 specular = numerator / max(denominator, 0.001);

        Lo += (kD * albedo.xyz / PI + specular) * radiance * NdotL * shadow;
    }
    float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    // -------------IBL Lighting-----------------------//
    float3 irradiance = irradianceMap.Sample(my_sampler, N).rgb;
    float3 indirectDiffuse = irradiance * albedo.xyz;
    const float MAX_REFLECTION_LOD = 4;
    float3 prefilteredColor = prefilterMap.SampleLevel(my_sampler, -R, roughness * MAX_REFLECTION_LOD).rgb;
    float2 envBRDF = BRDFLUT.Sample(Brdfsampler, float2(max(dot(N, V), 0.0), roughness)).rg;
    
    float3 indirectSpecular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    // ------------------------------------------------//

    
    float3 ambient = (kD * indirectDiffuse + indirectSpecular) * 1;
   
    float3 color = ambient + Lo;
    // Tonemap
    color = color / (color + float3(1.0f, 1.0f, 1.0f));

    // Gamma Inverse Correct
    float invGamma = 1.0f / 2.2f;
    color = pow(color, float3(invGamma, invGamma, invGamma));
    return float4(color, albedo.a);
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

float DirShadow(float3 projCoords, int layer, uint2 Offset, uint2 Size)
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
    float2 offset_uv = Offset / 4096.f.xx;
    float2 size_uv = Size / 8196.f.xx;
    projCoords.xy *= size_uv;
    float4 x = { offset_uv.x, offset_uv.x+size_uv.x, offset_uv.x, offset_uv.x+size_uv.x };
    float4 y = { offset_uv.y, offset_uv.x, offset_uv.y+size_uv.y, offset_uv.y+size_uv.y };
    float2 uv = float2(projCoords.x + x[layer], projCoords.y + y[layer]);
    
    float closestDepth1 = shadow_t.SampleCmpLevelZero(ShadowSampler, uv, currentDepth).r;
    return closestDepth1;
}
float SpotShadow(float3 projCoords, uint2 Offset, uint2 Size)
{
    projCoords.x = (projCoords.x + 1) * 0.5;
    projCoords.y = (1 - projCoords.y) * 0.5;
    projCoords.z = 0 + projCoords.z * (1);
    float currentDepth = projCoords.z;
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
    projCoords.xy *= Size / 4096.f.xx;
    projCoords.xy += Offset / 4096.f.xx;
    float closestDepth1 = shadow_t.SampleCmpLevelZero(ShadowSampler, projCoords.xy, currentDepth).r;
    return closestDepth1;
}
float Window(float distance, float max_distance)
{
    return pow(max((1 - pow((distance / max_distance), 4)), 0), 2);
}