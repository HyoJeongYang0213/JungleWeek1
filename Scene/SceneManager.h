#pragma once 

#include <vector>
#include <memory>

#include "IScene.hpp"
#include "../Renderer/IRenderer.hpp"

class SceneManager {
public:
	SceneManager(IRenderer& renderer);
	~SceneManager(); 

	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;

	SceneManager(SceneManager&&) = default;
	SceneManager& operator=(SceneManager&&) = default;

public:
	void NextScene();

	void Tick(float dt);
	void Render(IRenderer& renderer);

private:
	std::vector<std::unique_ptr<IScene>> mScenes;
	UINT mCurrentSceneIndex;
};