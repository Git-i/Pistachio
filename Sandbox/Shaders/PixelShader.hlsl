Texture2D BRDFLUT : register(t0);
TextureCube irradianceMap : register(t1);
TextureCube prefilterMap : register(t2);
Texture2D albedot : register(t3);
Texture2D roughnesst : register(t4);
Texture2D metalt : register(t5);
Texture2D aot : register(t6);
SamplerState my_sampler;

float  DistributionGGX(float3 N, float3 H, float roughness);
float  GeometrySchlickGGX(float NdotV, float roughness);
float  GeometrySmith(float3 N, float3 V, float3 L, float roughness);
float3 fresnelSchlick(float cosTheta, float3 F0);
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness);

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
};
cbuffer CBuff : register(b0)
{
    Light lights[128];
    float4 numlights;
};
cbuffer MaterialData : register(b1)
{
    float4 albedo;
    float metallic;
    float roughness;
    int entityID;
}

PS_OUTPUT main(float3 position : WORLD_POSITION, float3 Normal : FRAGMENT_NORMAL, float2 uv : UV, float3 camPos : V_POSITION)
{
    PS_OUTPUT pso;
    float3 WorldPos = position;
	
    float3 N = normalize(Normal);
    float3 V = normalize(camPos.xyz - WorldPos);
    float3 R = reflect(-V, N);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);
    
    // reflectance equation
    float3 Lo = float3(0.0, 0.0, 0.0);
    // Lighting
    for (int i = 0; i < numlights.x; i++)
    {
        float3 L;
        float attenuation;
        // calculate per-light radiance
        if(lights[i].positionxtype.w == 0)
            L = normalize(lights[i].positionxtype.xyz);
        else if (lights[i].positionxtype.w == 1)
            L = normalize(lights[i].positionxtype.xyz-WorldPos);
        float3 H = normalize(V + L);
        if(lights[i].positionxtype.w == 0)
            attenuation = lights[i].colorxintensity.w;
        else if (lights[i].positionxtype.w == 1)
        {
            float distance = length(lights[i].positionxtype.xyz - WorldPos);
            attenuation = lights[i].colorxintensity.w / (distance * distance);
        }
        float3 radiance = (lights[i].colorxintensity.xyz) * attenuation;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - metallic;

        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        float3 specular = numerator / denominator;

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    // -------------IBL Lighting-----------------------//
    float3 irradiance = irradianceMap.Sample(my_sampler, -N).rgb;
    float3 indirectDiffuse = irradiance * albedo;
    const float MAX_REFLECTION_LOD = 4.0;
    float3 prefilteredColor = prefilterMap.SampleLevel(my_sampler, -R, roughness * MAX_REFLECTION_LOD).rgb;
    float2 envBRDF = BRDFLUT.Sample(my_sampler, float2(-clamp(max(dot(N, V), 0.0), 0.0, 0.9), roughness)).rg;
    
    float3 indirectSpecular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    // ------------------------------------------------//

    float3 ambient = (kD * indirectDiffuse + indirectSpecular) * 1;
   
    float3 color = ambient + Lo;
	
    // Tonemap
    color = color / (color + float3(1.0f, 1.0f, 1.0f));

    // Gamma Inverse Correct
    float invGamma = 1.0f / 2.2f;
    color = pow(color, float3(invGamma, invGamma, invGamma));
    pso.color1 = float4(color, 1.0);
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