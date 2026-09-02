#include "EndingScene.h"
#include "../Renderer/Renderer.h"
#include "../UI/GameButtonUI.hpp"

EndingScene::EndingScene()
{
}

EndingScene::~EndingScene()
{
}

void EndingScene::Reset()
{
}

void EndingScene::Tick(float dt)
{
}

void EndingScene::Render(IRenderer& renderer)
{
	auto& concreteRenderer = static_cast<Renderer&>(renderer);
	concreteRenderer.Prepare();
}
