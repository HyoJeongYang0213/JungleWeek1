#pragma once 

#include "IScene.hpp"
#include "../Renderer/IRenderer.hpp"

class EndingScene : public IScene {
public:
	EndingScene();
	~EndingScene() override;
	EndingScene(const EndingScene&) = delete;
	EndingScene& operator=(const EndingScene&) = delete;
	EndingScene(EndingScene&&) = default;
	EndingScene& operator=(EndingScene&&) = default;

public:
	virtual SceneType GetSceneType() const override { return SceneType::Ending; }

	virtual void Reset() override;
	virtual void Tick(float dt) override;
	virtual void Render(IRenderer& renderer) override;
};