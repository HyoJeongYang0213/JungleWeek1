#pragma once 
#include "../Renderer/IRenderer.hpp"

class IScene {
public:
	virtual  ~IScene() = default;

public:
	virtual void Tick(float dt) = 0; 
	virtual void Render(IRenderer& renderer) = 0;

	template<typename T>
	T* As() {
		return static_cast<T*>(this);
	}
};