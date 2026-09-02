#pragma once 

#include <d3d11.h>


#include "../Resource/Primitive.hpp"
#include "../Utils/Math.hpp"

#include "../Renderer/IRenderer.hpp"

class Water
{
public:
	Water(ID3D11Buffer* vb, UINT numVertices);

#include <array>
#include "../Utils/Math.hpp"

#include "../Renderer/Renderer.h"

inline constexpr std::size_t WATER_SURFACE_SEGMENTS = 128;
inline constexpr std::size_t WATER_TRIANGLE_COUNT = WATER_SURFACE_SEGMENTS + 1;
inline constexpr std::size_t WATER_VERTEX_COUNT = WATER_TRIANGLE_COUNT * 3;

inline std::array<VertexTexture, WATER_VERTEX_COUNT> GenerateWaterVertices(float width, float height)
{
	std::array<VertexTexture, WATER_VERTEX_COUNT> vertices{};

	constexpr float Left = -1.0f;
	constexpr float Right = 1.0f;
	constexpr float Bottom = -1.0f;
	constexpr float Top = 1.0f;

	std::size_t index = 0;

	vertices[index++] = { Left, Bottom, 0.0f, 0.0f, 1.0f };
	vertices[index++] = { Right, Bottom, 0.0f, 1.0f, 1.0f };
	vertices[index++] = { Right, Top, 0.0f, 1.0f, 0.0f };

	for (std::size_t i = WATER_SURFACE_SEGMENTS; i > 0; --i)
	{
		const float t0 = static_cast<float>(i) / static_cast<float>(WATER_SURFACE_SEGMENTS);
		const float t1 = static_cast<float>(i - 1) / static_cast<float>(WATER_SURFACE_SEGMENTS);
		const float x0 = Left + (Right - Left) * t0;
		const float x1 = Left + (Right - Left) * t1;

		vertices[index++] = { Left, Bottom, 0.0f, 0.0f, 1.0f };
		vertices[index++] = { x0, Top, 0.0f, t0, 0.0f };
		vertices[index++] = { x1, Top, 0.0f, t1, 0.0f };
	}

	return vertices;
}


class Water {
public:
	Water(IRenderer& renderer, float Initheight);
	~Water();

	Water(const Water&) = delete;
	Water& operator=(const Water&) = delete;

	Water(Water&&) = default;
	Water& operator=(Water&&) = default;

public:
	void Tick(float t);
	bool IsGameOver() const;
	void Render(IRenderer& renderer);

private:
	ID3D11Buffer* mVertexBuffer;
	UINT mNumVertices;

	float mCenterX;
	float mBaseY;

	float mScaleX;
	void Start(); 

	void Tick(float dt);

	void Render(IRenderer& renderer);
public:
	float mHeight{}; 
	bool mIsActive{ false };
	float t{ 0.f };

	ID3D11Buffer* mVertexBuffer = nullptr;
	ID3D11ShaderResourceView* mSRVWater = nullptr; 

	ID3D11VertexShader* mWaterVertexShader = nullptr;
	ID3D11PixelShader* mWaterPixelShader = nullptr;
};