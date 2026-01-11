cbuffer PerFrame : register(b1)
{
    float3 lightDir;
    float3 lightColor;
    float3 ambientColor;
    float3 viewPos;
};

cbuffer PerInstance : register(b2)
{
    float4x4 modelMatrix;
    float4x4 normalMatrix;
    
    float4 diffuseColour;
    float Kd;
    float Ks;
    float shininess;
    bool hasDiffuseTex;
};

Texture2D colorTex : register(t0);
SamplerState colorSamp : register(s0);

static const float PI = 3.14159265f;

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

float4 main(float3 worldPosition : POSITION, float3 normal : NORMAL, float2 coord : TEXCOORD) : SV_TARGET
{
    float3 baseColor = diffuseColour.rgb;
    if (hasDiffuseTex)
        baseColor *= colorTex.Sample(colorSamp, coord).rgb;
    
    float3 N = normalize(normal);
    float3 L = normalize(lightDir);
    float3 V = normalize(viewPos - worldPosition);
    
    float NdotL = max(dot(N, L), 0.0f);
    if (NdotL <= 0.0f)
    {
        float3 ambOnly = ambientColor * baseColor;
        return float4(ambOnly, 1.0f);
    }
    
    float3 H = normalize(L + V);
    float NdotH = max(dot(N, H), 0.0f);
    float VdotH = max(dot(V, H), 0.0f);
    
    float3 F0 = 0.0f.xxx;
    F0 = lerp(F0, 1.0f.xxx, saturate(Ks));
    
    float3 kS = FresnelSchlick(VdotH, F0);
    float3 kD = 1.0f - kS;
    
    float3 diffuse = (kD * baseColor / PI) * NdotL;
    
    float specNorm = (shininess + 2.0f) / (2.0f * PI);
    float specPow = pow(NdotH, shininess);
    float3 specular = kS * (specNorm * specPow) * NdotL;
    
    float3 ambient = ambientColor * baseColor;
    float3 lit = (diffuse + specular) * lightColor;
    
    float3 color = ambient + lit;
    return float4(color, 1.0f);
}