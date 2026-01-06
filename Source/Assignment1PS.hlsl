Texture2D colorTex : register(t0);
SamplerState colorSampler : register(s0);
float4 main(float2 coords : TEXCOORD) : SV_TARGET
{
    return colorTex.Sample(colorSampler, coords);
}