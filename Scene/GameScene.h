#pragma once 

#include <d3d11.h>
#include <vector>
#include <memory>

#include "IScene.hpp"

#include "../Map/MapGlobals.hpp"
#include "../Map/PlatformTileManager.h"

#include "../Resource/Primitive.hpp"

#include "../Physics/Collider.h"

#include "../Player/Camera.h"
#include "../Player/Input.h"

#include "../Map/MapGenerator.h"
#include "../Map/Water.h"

#include "../Renderer/IRenderer.hpp"

class GameScene : public IScene {
public:
	GameScene(IRenderer& renderer);
	~GameScene() override;

	GameScene(const GameScene&) = delete;
	GameScene& operator=(const GameScene&) = delete;

	GameScene(GameScene&&) = default;
	GameScene& operator=(GameScene&&) = default;

public:
	virtual SceneType GetSceneType() const override { return SceneType::Game; }

	virtual void Reset() override;
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

	StaticCollider LeftWall{ MapGlobals::LEFT_BORDER_POINT, MapGlobals::LEFT_BORDER_NORMAL };
	StaticCollider RightWall{ MapGlobals::RIGHT_BORDER_POINT, MapGlobals::RIGHT_BORDER_NORMAL };
	StaticCollider TopWall{ MapGlobals::TOP_BORDER_POINT, MapGlobals::TOP_BORDER_NORMAL };
	StaticCollider BottomWall{ MapGlobals::BOTTOM_BORDER_POINT, MapGlobals::BOTTOM_BORDER_NORMAL };

	Camera mCamera{ Vector3(0.0f, 0.0f, 0.0f) };

	InfiniteMap mBackGround{};
	PlatformTileManager mPlatformTileManager{};

	ID3D11Buffer* mVertexBufferSphere = nullptr;

	ID3D11VertexShader* mTextureVertexShader = nullptr;
	ID3D11PixelShader* mTexturePixelShader = nullptr;
	ID3D11InputLayout* mTextureLayout = nullptr;
	ID3D11SamplerState* mSamplerState = nullptr;

	ID3D11Buffer* mBallVertexBuffer = nullptr;
	ID3D11ShaderResourceView* mSRVBall = nullptr;

	Input mInput{};

	std::unique_ptr<Water> mWater;
};
