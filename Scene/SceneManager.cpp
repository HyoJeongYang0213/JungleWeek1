#include "SceneManager.h"

#include "TitleScene.h"
#include "GameScene.h"
#include "EndingScene.h"

SceneManager::SceneManager(IRenderer& renderer)
{
	mScenes.emplace_back(std::make_unique<TitleScene>(renderer));
	mScenes.emplace_back(std::make_unique<GameScene>(renderer));
	mScenes.emplace_back(std::make_unique<EndingScene>(renderer));

	mCurrentSceneIndex = 0; 
}

SceneManager::~SceneManager()
{
	Shutdown();
}

void SceneManager::Shutdown()
{
	mScenes.clear();
}

void SceneManager::NextScene()
{
	auto nextSceneIndex = (mCurrentSceneIndex + 1) % mScenes.size();
	mScenes[nextSceneIndex]->Reset();
	mCurrentSceneIndex = nextSceneIndex;

}

void SceneManager::Tick(float dt)
{
	mScenes[mCurrentSceneIndex]->Tick(dt);
}

void SceneManager::Render(IRenderer& renderer)
{
	mScenes[mCurrentSceneIndex]->Render(renderer);
}
