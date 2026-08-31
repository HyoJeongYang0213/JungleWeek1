#pragma once 

#include <d3d11.h>
#include "../Physics/Math.hpp"

class IRenderer {
public:
	virtual ~IRenderer() = default;

	virtual void UpdateConstant(Vector3 Offset, float scale, float rotation) = 0;
	virtual void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) = 0;
};
