Texture2D BRDFLUT : register(t0);
TextureCube irradianceMap : register(t1);
TextureCube prefilterMap : register(t2);
Texture2D albedoTexture : register(t3);
Texture2D roughnessTexture : register(t4);
Texture2D metallicTexture : register(t5);
Texture2D normalTexture : register(t6);
Texture2D shadowMap[2] : register(t9);
SamplerState my_sampler :register(s0);
SamplerState Brdfsampler : register(s1);
SamplerState ShadowSampler : register(s2);

float  DistributionGGX(float3 N, float3 H, float roughness);
float  GeometrySchlickGGX(float NdotV, float roughness);
float  GeometrySmith(float3 N, float3 V, float3 L, float roughness);
float3 fresnelSchlick(float cosTheta, float3 F0);
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness);
float Shadow(float3 projCoords, int layer, float NdotL, int index);
float Window(float distance, float max_distance);
#define PI 3.14159265359
struct PS_OUTPUT
{
    float4 color1 : SV_Target0;
    int color2 : SV_Target1;
};
struct Light
{
    float4 positionxtype; // for directional lights this is direction and type
    float4 colorxintensity;
    float4 exData;
    float4 rotation;
};
cbuffer CBuff : register(b0)
{
    Light lights[128];
};
cbuffer MaterialData : register(b1)
{
    float4 albedoMultiplier;
    float metallicMultiplier;
    float roughnessMultiplier;
    int entityID;
}

struct PSINTPUT
{
    float3 WorldPos : WORLD_POSITION;
    float3 Normal : FRAGMENT_NORMAL;
    float2 uv : UV;
    float3 camPos : V_POSITION;
    float depthViewspace : SM_LAYER;
    int numlights : NUM_LIGHTS;
    float4 lightSpacePositions[16] : LS_POS;
};

PS_OUTPUT main(PSINTPUT input)
{
    PS_OUTPUT pso;
    float4 albedo = albedoMultiplier * albedoTexture.Sample(my_sampler, input.uv);
    float metallic = metallicMultiplier * metallicTexture.Sample(my_sampler, input.uv).r;
    float roughness = roughnessMultiplier * roughnessTexture.Sample(my_sampler, input.uv).r;
    clip(albedo.a < 0.1f ? -1 : 1);
    float3 N = normalize(input.Normal);
    float3 V = normalize(input.camPos.xyz - input.WorldPos);
    float3 R = reflect(-V, N);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo.xyz, metallic);
    float l;
    // reflectance equation
    float3 Lo = float3(0.0, 0.0, 0.0);
    // Lighting
    for (int i = 0; i < input.numlights; i++)
    {
        // -------------Evaluate L and Attenuation-----------------//
        float3 Ls[3] = { normalize(lights[i].rotation.xyz), normalize(lights[i].positionxtype.xyz - input.WorldPos), float3(0,0,0) };
        Ls[2] = Ls[1];
        float3 L = Ls[lights[i].positionxtype.w];
        
        float distance = length(lights[i].positionxtype.xyz - input.WorldPos);
        float window = Window(distance, lights[i].exData.z);
        float t = saturate((dot(lights[i].rotation.xyz, -L) - lights[i].exData.x) / (lights[i].exData.y - lights[i].exData.x));
        float attenuations[3] = { 1, window / (distance * distance), window * t * t / (distance * distance) };
        float attenuation = attenuations[lights[i].positionxtype.w];
        //-----------------------------------------------------------//
        float NdotL = dot(N, L);
        // -----------Shadow------------------------------//
        float shadow = 0.f;
        if (lights[i].exData.w)
        {
            int shadowMaplayer = 3;
            float cascadePlaneDistances[4] = { 10.f, 50.f, 70.f, 100.f };
            if (input.depthViewspace < cascadePlaneDistances[3])
            {
                shadowMaplayer = 3;
            }
            if (input.depthViewspace < cascadePlaneDistances[2])
            {
                shadowMaplayer = 2;
            }
            if (input.depthViewspace < cascadePlaneDistances[1])
            {
                shadowMaplayer = 1;
            }
            if (input.depthViewspace < cascadePlaneDistances[0])
            {
                shadowMaplayer = 0;
            }
            shadowMaplayer += i * 4;
            float4 lightSpacePos = input.lightSpacePositions[shadowMaplayer];
            shadow = Shadow(lightSpacePos.xyz / lightSpacePos.w, shadowMaplayer, NdotL, i);
        }
        if(shadow == 1.f)
            continue;
        // -----------------------------------------------//
        NdotL = max(NdotL, 0.0);
        float3 H = normalize(V + L);
        float3 radiance = (lights[i].colorxintensity.xyz) * attenuation;

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

        Lo += (kD * albedo.xyz / PI + specular) * radiance * NdotL * lights[i].colorxintensity.w * (1 - shadow);
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
    pso.color1 = float4(color,  albedo.a);
    pso.color2 = entityID;
    return pso; 
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta , 5.0);
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

float Shadow(float3 projCoords, int layer, float NdotL, int index)
{
    projCoords.x = (projCoords.x + 1) * 0.25;
    projCoords.y = (1 - projCoords.y) * 0.25;
    
    projCoords.z = 0 + projCoords.z * (1);
    float currentDepth = projCoords.z;
    if (currentDepth > 1.0)
    {
        return 0.0;
    }
    float x[4] = {0, 0.5, 0, 0.5 };
    float y[4] = { 0, 0, 0.5, 0.5};
    float closestDepth = shadowMap[index].Sample(ShadowSampler, float2(projCoords.x + x[layer], projCoords.y + y[layer])).r;
    float bias = max(0.05 * (1.0 - NdotL), 0.005);
    float cascadePlaneDistances[4] = { 10.f, 50.f, 70.f, 100.f };
    bias *= 1 / (cascadePlaneDistances[layer] * 0.5f);
    float shadow =  currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}
float Window(float distance, float max_distance)
{
    return pow(max((1 - pow((distance / max_distance), 4)), 0), 2);
}