cbuffer constants : register(b0)
{
    float3 Offset;
    float3 scale;
    float rotationAngle;
    float3x3 projectionMatrix;
}

Texture2D gTexture : register(t0);
SamplerState gSampler : register(s0);

struct VS_INPUT
{
    float4 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;

    float2 scaledPosition = float2(input.position.x * scale.x, input.position.y * scale.y);

    float s = sin(rotationAngle);
    float c = cos(rotationAngle);

    float2 rotatedPosition;
    rotatedPosition.x = scaledPosition.x * c - scaledPosition.y * s;
    rotatedPosition.y = scaledPosition.x * s + scaledPosition.y * c;

    float2 translatedPosition = rotatedPosition + float2(Offset.x, Offset.y);

    float3 projectedPosition = mul(projectionMatrix, float3(translatedPosition, 1.0f));

    output.position = float4(projectedPosition, 1.0f);

    output.texCoord = input.texCoord;

    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    float4 color = gTexture.Sample(gSampler, input.texCoord);

    clip(color.a - 0.05f);

    return color;
}