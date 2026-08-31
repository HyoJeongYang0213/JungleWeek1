// ShaderW0.hlsl
struct VS_INPUT
{
	float4 position : POSITION; // Input position from vertex buffer
	float4 color : COLOR;       // Input color from vertex buffer
};

struct PS_INPUT
{
	float4 position : SV_POSITION; // Transformed position to pass to the pixel shader
	float4 color : COLOR;          // Color to pass to the pixel shader
};

cbuffer constants : register(b0)
{
	float3 Offset;
	float scale;
	float rotationAngle;
	float3 pad; 
}

PS_INPUT mainVS(VS_INPUT input)
{
	PS_INPUT output;

	float2 scaledPosition = input.position.xy * scale;

	float s = sin(rotationAngle);
	float c = cos(rotationAngle);

	float2 rotatedPosition;
	rotatedPosition.x = scaledPosition.x * c - scaledPosition.y * s;
	rotatedPosition.y = scaledPosition.x * s + scaledPosition.y * c;

	output.position = float4(rotatedPosition + Offset.xy, input.position.z, 1.0f);

	output.color = input.color;

	return output;
}
float4 mainPS(PS_INPUT input) : SV_TARGET
{
	// Output the color directly
	return input.color;
}
