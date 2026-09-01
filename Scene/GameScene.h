#pragma once 

#include <vector>
#include <memory>
#include <d3d11.h>

#include "IScene.hpp"

#include "../Map/MapGlobals.hpp"

#include "../Resource/Primitive.hpp"

#include "../Physics/Collider.h"

#include "../Player/Camera.h"

#include "../Map/MapGenerator.h"

class GameScene : public IScene {
public:
	GameScene() = default;
	~GameScene() override = default;

	GameScene(const GameScene&) = delete;
	GameScene& operator=(const GameScene&) = delete;

	GameScene(GameScene&&) = default;
	GameScene& operator=(GameScene&&) = default;

public:
	virtual void Tick(float dt) override;
	virtual void Render(IRenderer& renderer, ID3D11SamplerState* pSamplerState) override;

	template <typename T, typename... Args>
	inline bool CreatePrimitive(Args&&... args)
	{
		static_assert(std::is_base_of<Primitive, T>::value, "T must be derived from Primitive");
		auto primitive = std::make_unique<T>(std::forward<Args>(args)...);
		if (!primitive)
		{
			return false;
		}
		mPrimitives.emplace_back(std::move(primitive));
		return true;
	}

private:
	std::vector<std::unique_ptr<Primitive>> mPrimitives{};
	
	StaticCollider LeftWall{ MapGlobals::LEFT_BORDER_POINT, MapGlobals::LEFT_BORDER_NORMAL };
	StaticCollider RightWall{ MapGlobals::RIGHT_BORDER_POINT, MapGlobals::RIGHT_BORDER_NORMAL };
	StaticCollider TopWall{ MapGlobals::TOP_BORDER_POINT, MapGlobals::TOP_BORDER_NORMAL };
	StaticCollider BottomWall{ MapGlobals::BOTTOM_BORDER_POINT, MapGlobals::BOTTOM_BORDER_NORMAL };

	Camera mCamera{ Vector3(0.0f, 0.0f, 0.0f) };

	InfiniteMap mBackGround{}; 
};