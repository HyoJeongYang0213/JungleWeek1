#include "InfiniteMap.h"
#include <cmath>

void InfiniteMap::Init(Renderer& renderer, ID3D11ShaderResourceView* groundmShaderResourceView, const std::vector<ID3D11ShaderResourceView*>& pattermShaderResourceViews)
{
	mShaderResourceViewGround = groundmShaderResourceView;
	mShaderResourceViewPatterns = pattermShaderResourceViews;

	mFloorTextures[0] = mShaderResourceViewGround;

	mVertexBufferChunk = renderer.CreateDynamicVertexBuffer(sizeof(VertexTex) * 6);

}

InfiniteMap::~InfiniteMap()
{
	if(mVertexBufferChunk)
	{
		mVertexBufferChunk->Release();
		mVertexBufferChunk = nullptr;
	}
}

ID3D11ShaderResourceView* InfiniteMap::GetOrCreateFloorTexture(int floorIndex)
{
	auto It = mFloorTextures.find(floorIndex);
	if (It != mFloorTextures.end())
	{
		return It->second;
	}
	
	if (!mShaderResourceViewPatterns.empty())
	{
		int MaxIndex = (int)(mShaderResourceViewPatterns.size()) - 1;
		int RandIndex = Rnd::GetRandomInt(0, MaxIndex);

		mFloorTextures[floorIndex] = mShaderResourceViewPatterns[RandIndex];
		return mShaderResourceViewPatterns[RandIndex];
	}

	return mShaderResourceViewGround;
}

void InfiniteMap::DrawChunk(Renderer& renderer, int floorIndex, float cameraCenterY)
{
	ID3D11ShaderResourceView* Texture = GetOrCreateFloorTexture(floorIndex);
	if (!Texture) return;

	float TopY = -(float)(floorIndex)*Globals::MAP_HEIGHT;
	float BottomY = TopY + Globals::MAP_HEIGHT;

	float HalfView = Globals::VIEW_HEIGHT_PX * 0.5f;
	float NDCTop = (cameraCenterY - TopY) / HalfView;
	float NDCBottom = (cameraCenterY - BottomY) / HalfView;

	if (NDCBottom > 1.0f || NDCTop < -1.0f)
	{
		return;
	}

	VertexTex quad[6] = {
		{ -1.0f, NDCTop,    0.0f,  0.0f, 0.0f },
		{  1.0f, NDCTop,    0.0f,  1.0f, 0.0f },
		{  1.0f, NDCBottom, 0.0f,  1.0f, 1.0f },

		{ -1.0f, NDCTop,    0.0f,  0.0f, 0.0f },
		{  1.0f, NDCBottom, 0.0f,  1.0f, 1.0f },
		{ -1.0f, NDCBottom, 0.0f,  0.0f, 1.0f }
	};

	renderer.UpdateDynamicVertexBuffer(mVertexBufferChunk, quad, sizeof(quad));

	UINT stride = sizeof(VertexTex);
	UINT offset = 0;
	renderer.DeviceContext->IASetVertexBuffers(0, 1, &mVertexBufferChunk, &stride, &offset);
	renderer.DeviceContext->PSSetShaderResources(0, 1, &Texture);
	renderer.DeviceContext->Draw(6, 0);
}

void InfiniteMap::Render(Renderer& renderer, ID3D11SamplerState* sampler, float cameraCenterY)
{
	renderer.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	renderer.DeviceContext->PSSetSamplers(0, 1, &sampler);

	float viewTopY = cameraCenterY - (Globals::VIEW_HEIGHT_PX * 0.5f);
	float viewBottomY = cameraCenterY + (Globals::VIEW_HEIGHT_PX * 0.5f);

	int minFloor = static_cast<int>(std::floor((Globals::MAP_HEIGHT - viewBottomY) / Globals::MAP_HEIGHT));
	int maxFloor = static_cast<int>(std::floor((Globals::MAP_HEIGHT - viewTopY) / Globals::MAP_HEIGHT));

	if (minFloor < 0) minFloor = 0;

	for (int f = minFloor; f <= maxFloor; ++f)
	{
		DrawChunk(renderer, f, cameraCenterY);
	}
}