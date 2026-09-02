#pragma once 
#include <d3d11.h>

#include "../Renderer/IRenderer.hpp"

class IScene {
public:
	virtual  ~IScene() = default;

public:
	virtual void Tick(float dt) = 0; 
	virtual void Render(IRenderer& renderer, ID3D11SamplerState* pSamplerState) = 0;

	template<typename T>
	T* As() {
		return static_cast<T*>(this);
	}
};