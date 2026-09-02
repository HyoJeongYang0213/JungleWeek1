#include "TitleScene.h"
#include "../Renderer/Renderer.h"

TitleScene::TitleScene()
{
}

TitleScene::~TitleScene()
{
}

void TitleScene::Reset()
{
}

void TitleScene::Tick(float dt)
{
}

void TitleScene::Render(IRenderer& renderer)
{
	auto& concreteRenderer = static_cast<Renderer&>(renderer);
	concreteRenderer.Prepare(); 


}
