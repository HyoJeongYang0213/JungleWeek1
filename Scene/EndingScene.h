#pragma once 

#include "IScene.hpp"
#include "../Renderer/IRenderer.hpp"

class EndingScene : public IScene {
public:
	EndingScene(IRenderer& renderer);
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

private:
	ID3D11VertexShader* mTextureVertexShader = nullptr;
	ID3D11PixelShader* mTexturePixelShader = nullptr;
	ID3D11InputLayout* mTextureLayout = nullptr;
	ID3D11SamplerState* mSamplerState = nullptr;

	ID3D11Buffer* mQuadVertexBuffer = nullptr;
	ID3D11ShaderResourceView* mSRVEndingBG = nullptr;
	ID3D11ShaderResourceView* mSRVEndingText = nullptr;
	ID3D11ShaderResourceView* mScore = nullptr;
};