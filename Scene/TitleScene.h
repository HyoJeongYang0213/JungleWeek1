#pragma once 

#include "IScene.hpp"
#include "../Renderer/IRenderer.hpp"

class TitleScene : public IScene {
public:
	TitleScene();
	~TitleScene() override;
	TitleScene(const TitleScene&) = delete;
	TitleScene& operator=(const TitleScene&) = delete;
	TitleScene(TitleScene&&) = default;
	TitleScene& operator=(TitleScene&&) = default;

public:
	virtual SceneType GetSceneType() const override { return SceneType::Title; }

	virtual void Reset() override;
	virtual void Tick(float dt) override;
	virtual void Render(IRenderer& renderer) override;
};