#include "Water.h"

#include "../Renderer/Renderer.h"
#include "../Map/TextureLoader.hpp"
#include "../Audio/SoundManager.h"

#include "../Map/MapGlobals.hpp"

Water::Water(IRenderer& renderer, float Initheight)
{
	Renderer& concreteRenderer = static_cast<Renderer&>(renderer);

	mVertexBuffer = concreteRenderer.CreateDynamicVertexBuffer(sizeof(VertexTexture) * WATER_VERTEX_COUNT);
	concreteRenderer.UpdateDynamicVertexBuffer(mVertexBuffer, GenerateWaterVertices(15.0f, 1.0f).data(), sizeof(VertexTexture) * WATER_VERTEX_COUNT);

	mSRVWater = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Water.png");

	ID3DBlob* VertexShaderBlob = nullptr, * psBlob = nullptr;

	D3DCompileFromFile(L"Resource/Shader/Water.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &VertexShaderBlob, nullptr);
	concreteRenderer.Device->CreateVertexShader(VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), nullptr, &mWaterVertexShader);

	D3DCompileFromFile(L"Resource/Shader/Water.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &psBlob, nullptr);
	concreteRenderer.Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mWaterPixelShader);
	VertexShaderBlob->Release();
	psBlob->Release();

}

Water::~Water()
{
	if (mVertexBuffer != nullptr) {
		mVertexBuffer->Release();
		mVertexBuffer = nullptr;

	}
	if (mSRVWater != nullptr) {
		mSRVWater->Release();
		mSRVWater = nullptr;
	}
	if (mWaterVertexShader != nullptr) {
		mWaterVertexShader->Release();
		mWaterVertexShader = nullptr;
	}
	if (mWaterPixelShader != nullptr) {
		mWaterPixelShader->Release();
		mWaterPixelShader = nullptr;
	}
}

void Water::Tick(float dt)
{
	t += dt;

	if (t > 0.f) {
		WaterGlobals::WATER_Y_SCALE += WaterGlobals::WATER_SPEED * dt;
		if (IsGameOver())
		{
			SoundManager::GetInstance().PlaySound("GameOver");
			WaterGlobals::B_GAME_OVER = true;
			OutputDebugStringA("게임오버!\n");
		}
	}
}
void Water::Reset()
{
	t = -10.f;
	WaterGlobals::WATER_Y_SCALE = 0.f;
}


bool Water::IsGameOver() const
{
	if (t > 0.f) {

		const float wSurfaceY = WaterGlobals::WATER_Y_SCALE;

		if (PlayerGlobals::PLAYERLOCATION.y - PlayerGlobals::PLAYERBALL->GetRadius() <= wSurfaceY)
		{
			return true;
		}
		return false;
	}

	return false;
}


void Water::Start()
{
	mIsActive = true;
}

void Water::Render(IRenderer& renderer)
{
	Renderer& concreteRenderer = static_cast<Renderer&>(renderer);

	concreteRenderer.DeviceContext->VSSetShader(mWaterVertexShader, nullptr, 0);
	concreteRenderer.DeviceContext->PSSetShader(mWaterPixelShader, nullptr, 0);

	concreteRenderer.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Vector3 Center = { MapGlobals::RIGHT_BORDER / 2.0f, 0.f, 0.0f };
	Vector3 HalfExtents = { MapGlobals::RIGHT_BORDER / 2.0f ,WaterGlobals::WATER_Y_SCALE, t };
	
	concreteRenderer.UpdateConstant(Center, HalfExtents, 0.0f);
	concreteRenderer.DeviceContext->PSSetShaderResources(0, 1, &mSRVWater);

	UINT Stride = sizeof(VertexTexture);
	UINT Offset = 0;
	
	concreteRenderer.SetAlphaBlendState(true);

	concreteRenderer.DeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &Stride, &Offset);
	concreteRenderer.DeviceContext->Draw(WATER_VERTEX_COUNT, 0);

	concreteRenderer.SetAlphaBlendState(false);
}
