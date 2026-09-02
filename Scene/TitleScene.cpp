#include "TitleScene.h"
#include "../Renderer/Renderer.h"
#include "../Map/TextureLoader.hpp"

TitleScene::TitleScene(IRenderer& renderer)
{
	Renderer& concreteRenderer = static_cast<Renderer&>(renderer);

	ID3DBlob* VertexShaderBlob = nullptr, * psBlob = nullptr;

	D3DCompileFromFile(L"Resource/Shader/SSQuad.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &VertexShaderBlob, nullptr);
	concreteRenderer.Device->CreateVertexShader(VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), nullptr, &mTextureVertexShader);

	D3DCompileFromFile(L"Resource/Shader/SSQuad.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &psBlob, nullptr);
	concreteRenderer.Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mTexturePixelShader);

	D3D11_INPUT_ELEMENT_DESC TextureLayoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	concreteRenderer.Device->CreateInputLayout(TextureLayoutDesc, 2, VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), &mTextureLayout);
	VertexShaderBlob->Release();
	psBlob->Release();

	mSamplerState = TextureLoader::CreateSamplerState(concreteRenderer.Device);

	mSRVTitle = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/main.png");

    VertexTexture quad[6] = {
    { -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
    {  1.0f,  1.0f, 0.0f,  1.0f, 0.0f },
    {  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },

    { -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
    {  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },
    { -1.0f, -1.0f, 0.0f,  0.0f, 1.0f }
    };

	mQuadVertexBuffer = concreteRenderer.CreateDynamicVertexBuffer(sizeof(quad));
    
	concreteRenderer.UpdateDynamicVertexBuffer(mQuadVertexBuffer, quad, sizeof(quad));
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
	Renderer& concreteRenderer = static_cast<Renderer&>(renderer);

	concreteRenderer.Prepare();

	concreteRenderer.DeviceContext->VSSetShader(mTextureVertexShader, nullptr, 0);
	concreteRenderer.DeviceContext->PSSetShader(mTexturePixelShader, nullptr, 0);
	concreteRenderer.DeviceContext->IASetInputLayout(mTextureLayout);
	concreteRenderer.DeviceContext->PSSetSamplers(0, 1, &mSamplerState);
	concreteRenderer.DeviceContext->VSSetConstantBuffers(0, 1, &concreteRenderer.ConstantBuffer);
	concreteRenderer.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Vector3 Center = { 7.5f,15.f, 0.0f };
	Vector3 HalfExtents = { 7.5f ,15.f, 0.0f };

	concreteRenderer.UpdateConstantIgnoreCamera(Center, HalfExtents, 0.0f);
	concreteRenderer.DeviceContext->PSSetShaderResources(0, 1, &mSRVTitle);

	UINT Stride = sizeof(VertexTexture);
	UINT Offset = 0;
	concreteRenderer.DeviceContext->IASetVertexBuffers(0, 1, &mQuadVertexBuffer, &Stride, &Offset);
	concreteRenderer.DeviceContext->Draw(6, 0);

}
