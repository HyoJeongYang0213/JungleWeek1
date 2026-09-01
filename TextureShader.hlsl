Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

struct VS_INPUT
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = float4(input.pos, 1.0f);
    output.uv = input.uv;
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    return g_Texture.Sample(g_Sampler, input.uv);
}