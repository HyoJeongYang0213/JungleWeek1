#pragma once 

#include <vector>
#include <memory>

#include "IScene.hpp"

#include "../Map/MapGlobals.hpp"

#include "../Resource/Primitive.hpp"

#include "../Physics/Collider.h"

#include "../Player/Camera.h"

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
	virtual void Render(IRenderer& renderer) override;

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
	
	StaticCollider LeftWall{ Globals::LEFT_BORDER_POINT, Globals::LEFT_BORDER_NORMAL };
	StaticCollider RightWall{ Globals::RIGHT_BORDER_POINT, Globals::RIGHT_BORDER_NORMAL };
	StaticCollider TopWall{ Globals::TOP_BORDER_POINT, Globals::TOP_BORDER_NORMAL };
	StaticCollider BottomWall{ Globals::BOTTOM_BORDER_POINT, Globals::BOTTOM_BORDER_NORMAL };
};