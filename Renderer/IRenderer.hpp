#pragma once 

#include <d3d11.h>
#include "../Utils/Math.hpp"

class IRenderer {
public:
	virtual ~IRenderer() = default;

	virtual void UpdateConstant(Vector3 Offset, Vector3 scale, float rotation) = 0;
	virtual void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) = 0;
	virtual void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices, UINT vertexStride) = 0;
};
