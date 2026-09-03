#pragma once 

#include <d3d11.h>

#include "IScene.hpp"
#include "../Renderer/IRenderer.hpp"

class TitleScene : public IScene {
public:
	TitleScene(IRenderer& renderer);
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

private:
	ID3D11VertexShader* mTextureVertexShader = nullptr;
	ID3D11PixelShader* mTexturePixelShader = nullptr;
	ID3D11InputLayout* mTextureLayout = nullptr;
	ID3D11SamplerState* mSamplerState = nullptr;

	ID3D11Buffer* mQuadVertexBuffer = nullptr;
	ID3D11ShaderResourceView* mSRVTitle = nullptr;
	
	ID3D11ShaderResourceView* mSRVLogo = nullptr;
};
