Texture2D gTexture : register(t0); // 현재 층 테마 (하단)
Texture2D gTextureNext : register(t1); // 다음 층 테마 (상단)
SamplerState gSampler : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float4 colA = gTexture.Sample(gSampler, input.texCoord);
    float4 colB = gTextureNext.Sample(gSampler, input.texCoord);

    // 아래쪽은 colA(현재), 위쪽으로 갈수록 colB(다음)
    float t = 1.0f - input.texCoord.y;
    float4 finalColor = lerp(colA, colB, t);

    clip(finalColor.a - 0.05f);
    return finalColor;
}