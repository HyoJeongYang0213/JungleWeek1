#pragma once 
#include <d3d11.h>

#include "../Renderer/IRenderer.hpp"

enum class SceneType {
	None,
	Game,
	Title,
	Ending,
	END
};

class IScene {
public:
	virtual  ~IScene() = default;

public:
	virtual SceneType GetSceneType() const = 0;

	virtual void Reset() = 0;
	virtual void Tick(float dt) = 0; 
	virtual void Render(IRenderer& renderer) = 0;

	template<typename T>
	T* As() {
		return static_cast<T*>(this);
	}
};