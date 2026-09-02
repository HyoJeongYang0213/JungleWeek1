#pragma once 

#include <d3d11.h>


#include "../Resource/Primitive.hpp"
#include "../Utils/Math.hpp"

#include "../Renderer/IRenderer.hpp"

class Water
{
public:
	Water(ID3D11Buffer* vb, UINT numVertices);

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
};