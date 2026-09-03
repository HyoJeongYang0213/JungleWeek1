#include "PlatformTileManager.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <chrono>

#include "MapGlobals.hpp"
#include "TextureLoader.hpp"
#include "../Physics/Collider.h"
#include "../Physics/CollisionDetector.h"
#include "../Renderer/Renderer.h"
#include "../Resource/vertexSimple.hpp"

namespace
{
	constexpr std::array<const char*, 9> PlatformPolygonImagePaths
	{
		"Asset/Clouds/Cloud-2.png",
		"Asset/Clouds/Cloud-5.png",
		"Asset/Clouds/Cloud-7.png",
		"Asset/Clouds/Cloud-10.png",
		"Asset/Clouds/Cloud-21.png",
		"Asset/Clouds/Cloud-23.png",
		"Asset/Clouds/Cloud-25.png",
		"Asset/Clouds/Cloud-31.png",
		"Asset/Clouds/Cloud-43.png"
	};

	constexpr float TileHalfWidth = 5.0f;
	constexpr float TileHalfHeight = 5.0f;
	constexpr float TileWidth = TileHalfWidth * 2.0f;
	constexpr float TileHeight = TileHalfHeight * 2.0f;

	constexpr float MinimumPolygonThickness = 1.0f;
	constexpr float MinimumPolygonGap = 1.6f;
	constexpr float TileBoundaryClearance = MinimumPolygonGap * 0.5f;
	constexpr float MinimumPolygonRotation = 0.0698132f;
	constexpr float MaximumPolygonRotation = 0.174533f;
	constexpr float MaximumVerticalJitter = 0.5f;
	constexpr float LoadDistanceBelow = 25.0f;
	constexpr float LoadDistanceAbove = 35.0f;

	Vector3 CalculateMaximumRotatedHalfExtents(const Vector3& scale, const Vector3& localHalfExtents)
	{
		const float cosine = std::cos(MaximumPolygonRotation);
		const float sine = std::sin(MaximumPolygonRotation);
		const float scaledHalfWidth = scale.x * localHalfExtents.x;
		const float scaledHalfHeight = scale.y * localHalfExtents.y;
		return
		{
			scaledHalfWidth * cosine + scaledHalfHeight * sine,
			scaledHalfWidth * sine + scaledHalfHeight * cosine,
			0.0f
		};
	}

	std::uint32_t MixBits(std::uint32_t value)
	{
		value ^= value >> 16;
		value *= 0x7FEB352Du;
		value ^= value >> 15;
		value *= 0x846CA68Bu;
		value ^= value >> 16;
		return value;
	}
}

PlatformTileManager::~PlatformTileManager()
{
	mPolygons.clear();
	ReleasePolygonImages();
}

void PlatformTileManager::Init(Renderer& renderer, std::uint32_t seed)
{
	Reset();
	mSeed = seed;
	mMinimumPolygonScale = 1.0f;
	mPolygonLocalHalfExtents = {};
	LoadPolygonImages(renderer);
	if (mPolygonImages.empty())
	{
		return;
	}

	Update(MapGlobals::VIEW_HEIGHT * 0.5f);
}

void PlatformTileManager::Update(float cameraCenterY)
{
	if (mPolygonImages.empty())
	{
		return;
	}

	static const auto StartTime = std::chrono::steady_clock::now();
	const auto Now = std::chrono::steady_clock::now();

	for (auto& p : mPolygons)
	{
		auto& curr = p.Polygon->GetRigidBody().GetPosition();
		float second = std::chrono::duration<float>(Now - StartTime).count();
		p.Polygon->SetPosition({ curr.x, curr.y - sinf(std::chrono::duration<float>(Now - StartTime).count()) * 0.005f, 0.f });
	}

	const float minimumY = std::max(0.0f, cameraCenterY - LoadDistanceBelow);
	const float maximumY = cameraCenterY + LoadDistanceAbove;
	const int minimumRow = std::max(0, static_cast<int>(std::floor(minimumY / TileHeight)));
	const int maximumRow = std::max(minimumRow, static_cast<int>(std::floor(maximumY / TileHeight)));
	if (minimumRow == mLastMinRow && maximumRow == mLastMaxRow)
	{
		return;
	}

	const int columnCount = static_cast<int>(std::ceil(MapGlobals::WORLD_WIDTH / TileWidth));
	std::unordered_set<TileCoordinate, TileCoordinateHash> requiredTiles{};
	for (int row = minimumRow; row <= maximumRow; ++row)
	{
		for (int column = 0; column < columnCount; ++column)
		{
			requiredTiles.insert({ column, row });
		}
	}

	mPolygons.erase(std::remove_if(mPolygons.begin(), mPolygons.end(), [&requiredTiles](const ActivePolygon& polygon){return requiredTiles.find(polygon.Coordinate) == requiredTiles.end();}), mPolygons.end());
	for (const TileCoordinate& coordinate : requiredTiles)
	{
		if (mActiveTiles.find(coordinate) == mActiveTiles.end()) CreateTileInstance(coordinate);
	}

	mActiveTiles = std::move(requiredTiles);
	mLastMinRow = minimumRow;
	mLastMaxRow = maximumRow;
}

void PlatformTileManager::FindCollisions(SphereCollider& sphere, std::vector<CollisionManifold>& manifolds)
{
	for (ActivePolygon& polygon : mPolygons)
	{
		CollisionManifold manifold{};
		PolygonCollider& collider = static_cast<PolygonCollider&>(polygon.Polygon->GetCollider());

		if (CollisionDetector::FindCollision(sphere, collider, manifold))
		{
			manifolds.emplace_back(manifold);
		}
	}
}

void PlatformTileManager::Render(IRenderer& renderer)
{
	if (mPolygonImages.empty())
	{
		return;
	}

	Renderer& concreteRenderer = static_cast<Renderer&>(renderer);
	concreteRenderer.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (ActivePolygon& polygon : mPolygons)
	{
		ID3D11ShaderResourceView* shaderResourceView = mPolygonImages[polygon.ResourceIndex].ShaderResourceView;
		concreteRenderer.DeviceContext->PSSetShaderResources(0, 1, &shaderResourceView);
		polygon.Polygon->Render(renderer);
	}
}

void PlatformTileManager::Reset()
{
	mPolygons.clear();
	mActiveTiles.clear();
	mLastMinRow = 1;
	mLastMaxRow = 0;
}

std::size_t PlatformTileManager::TileCoordinateHash::operator()(const TileCoordinate& coordinate) const
{
	const std::uint32_t column = static_cast<std::uint32_t>(coordinate.Column);
	const std::uint32_t row = static_cast<std::uint32_t>(coordinate.Row);
	return static_cast<std::size_t>(MixBits(column ^ (MixBits(row) + 0x9E3779B9u)));
}

void PlatformTileManager::LoadPolygonImages(Renderer& renderer)
{
	ReleasePolygonImages();
	mPolygonImages.reserve(PlatformPolygonImagePaths.size());

	for (const char* imagePath : PlatformPolygonImagePaths)
	{
		PolygonImageData imageData = PPolygon::CreateGeometryFromImage(imagePath);
		if (imageData.Vertices.empty() || imageData.Positions.size() < 3)
		{
			continue;
		}

		PolygonImageResource resource{};
		resource.VertexBuffer = renderer.CreateVertexBuffer(reinterpret_cast<VertexSimple*>(imageData.Vertices.data()), static_cast<UINT>(imageData.Vertices.size() * sizeof(VertexTexture)));
		resource.ShaderResourceView = TextureLoader::CreateTextureFromFile(renderer.Device, imagePath);
		if (!resource.VertexBuffer || !resource.ShaderResourceView)
		{
			if (resource.VertexBuffer) resource.VertexBuffer->Release();
			if (resource.ShaderResourceView) resource.ShaderResourceView->Release();
			continue;
		}

		resource.NumVertices = imageData.NumVertices;
		resource.Positions = std::move(imageData.Positions);
		float minimumY = (std::numeric_limits<float>::max)();
		float maximumY = (std::numeric_limits<float>::lowest)();
		Vector3 localHalfExtents{};
		for (const Vector3& position : resource.Positions)
		{
			minimumY = (std::min)(minimumY, position.y);
			maximumY = (std::max)(maximumY, position.y);
			localHalfExtents.x = (std::max)(localHalfExtents.x, std::abs(position.x));
			localHalfExtents.y = (std::max)(localHalfExtents.y, std::abs(position.y));
		}

		const float height = maximumY - minimumY;
		mMinimumPolygonScale = (std::max)(mMinimumPolygonScale, height > 0.0f ? MinimumPolygonThickness / height : 1.0f);
		mPolygonLocalHalfExtents.x = (std::max)(mPolygonLocalHalfExtents.x, localHalfExtents.x);
		mPolygonLocalHalfExtents.y = (std::max)(mPolygonLocalHalfExtents.y, localHalfExtents.y);
		mPolygonImages.push_back(std::move(resource));
	}
}

void PlatformTileManager::ReleasePolygonImages()
{
	for (PolygonImageResource& resource : mPolygonImages)
	{
		if (resource.VertexBuffer) resource.VertexBuffer->Release();
		if (resource.ShaderResourceView) resource.ShaderResourceView->Release();
	}

	mPolygonImages.clear();
}

void PlatformTileManager::CreateTileInstance(const TileCoordinate& coordinate)
{
	if (mPolygonImages.empty())
	{
		return;
	}

	const std::size_t pointCount = static_cast<std::size_t>(CreateRandomValue(coordinate, 0, 0x9E3779B9u) % 3u);
	if (pointCount == 0)
	{
		return;
	}

	const Vector3 tileCenter
	{
		MapGlobals::LEFT_BORDER + TileHalfWidth + static_cast<float>(coordinate.Column) * TileWidth,
		TileHalfHeight + static_cast<float>(coordinate.Row) * TileHeight,
		0.0f
	};
	const float maximumScale = CalculateMaximumScale(pointCount);
	const float minimumScale = (std::min)(maximumScale, (std::max)(2.8f, mMinimumPolygonScale));
	const Vector3 rotatedHalfExtents = CalculateMaximumRotatedHalfExtents(Vector3{ maximumScale, maximumScale, 0.0f }, mPolygonLocalHalfExtents);
	const float maximumCenterX = (std::max)(0.0f, TileHalfWidth - TileBoundaryClearance - rotatedHalfExtents.x);

	for (std::size_t pointIndex = 0; pointIndex < pointCount; ++pointIndex)
	{
		const std::size_t resourceIndex = static_cast<std::size_t>(CreateRandomValue(coordinate, pointIndex, 0x165667B1u)) % mPolygonImages.size();
		const PolygonImageResource& resource = mPolygonImages[resourceIndex];
		const float scale = CreateRandomFloat(coordinate, pointIndex, 0x27D4EB2Du, minimumScale, maximumScale);
		const std::uint32_t rotationValue = CreateRandomValue(coordinate, pointIndex, 0x85EBCA6Bu);
		const float rotationDirection = (rotationValue & 1u) == 0u ? -1.0f : 1.0f;
		const float rotation = rotationValue % 3u == 0u ? 0.0f : CreateRandomFloat(coordinate, pointIndex, 0xC2B2AE35u, MinimumPolygonRotation, MaximumPolygonRotation) * rotationDirection;
		const float centerX = CreateRandomFloat(coordinate, pointIndex, 0xD3A2646Cu, -maximumCenterX, maximumCenterX);
		float centerY{};
		if (pointCount == 1)
		{
			const float maximumCenterY = (std::max)(0.0f, TileHalfHeight - TileBoundaryClearance - rotatedHalfExtents.y);
			centerY = CreateRandomFloat(coordinate, pointIndex, 0xFD7046C5u, -maximumCenterY, maximumCenterY);
		}
		else
		{
			const float jitter = CreateRandomFloat(coordinate, pointIndex, 0xB55A4F09u, 0.0f, MaximumVerticalJitter);
			centerY = pointIndex == 0 ? -TileHalfHeight + TileBoundaryClearance + rotatedHalfExtents.y + jitter : TileHalfHeight - TileBoundaryClearance - rotatedHalfExtents.y - jitter;
		}

		const Vector3 worldCenter = tileCenter + Vector3{ centerX, centerY, 0.0f };
		auto polygon = std::make_unique<PPolygon>(resource.VertexBuffer, resource.NumVertices, worldCenter, Vector3{ scale, scale, 0.0f }, resource.Positions, true, static_cast<UINT>(sizeof(VertexTexture)));
		polygon->GetRigidBody().SetRotation(rotation);

		mPolygons.emplace_back(ActivePolygon{ coordinate, resourceIndex, std::move(polygon) });
	}
}

float PlatformTileManager::CalculateMaximumScale(std::size_t pointCount) const
{
	const Vector3 unitRotatedHalfExtents = CalculateMaximumRotatedHalfExtents(Vector3{ 1.0f, 1.0f, 0.0f }, mPolygonLocalHalfExtents);
	const float maximumHorizontalScale = (TileHalfWidth - TileBoundaryClearance) / unitRotatedHalfExtents.x;
	const float maximumRotatedHalfHeight = pointCount == 1 ? TileHalfHeight - TileBoundaryClearance : (TileHeight - MinimumPolygonGap - TileBoundaryClearance * 2.0f - MaximumVerticalJitter * 2.0f) * 0.25f;
	const float maximumVerticalScale = maximumRotatedHalfHeight / unitRotatedHalfExtents.y;
	return (std::min)(4.0f, (std::min)(maximumHorizontalScale, maximumVerticalScale));
}

std::uint32_t PlatformTileManager::CreateRandomValue(const TileCoordinate& coordinate, std::size_t pointIndex, std::uint32_t salt) const
{
	const std::uint32_t column = static_cast<std::uint32_t>(coordinate.Column);
	const std::uint32_t row = static_cast<std::uint32_t>(coordinate.Row);
	return MixBits(mSeed ^ MixBits(column + 0x9E3779B9u) ^ MixBits(row + 0x85EBCA6Bu) ^ MixBits(static_cast<std::uint32_t>(pointIndex) + salt));
}

float PlatformTileManager::CreateRandomFloat(const TileCoordinate& coordinate, std::size_t pointIndex, std::uint32_t salt, float minimum, float maximum) const
{
	const float ratio = static_cast<float>(CreateRandomValue(coordinate, pointIndex, salt) & 0x00FFFFFFu) / static_cast<float>(0x00FFFFFFu);
	return minimum + (maximum - minimum) * ratio;
}
