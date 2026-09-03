#pragma once

#include <cstddef>
#include <cstdint>
#include <d3d11.h>
#include <memory>
#include <unordered_set>
#include <vector>

#include "Polygon.h"
#include "../Physics/CollisionManifold.hpp"

class IRenderer;
class Renderer;
class SphereCollider;

class PlatformTileManager
{
public:
	PlatformTileManager() = default;
	~PlatformTileManager();

	PlatformTileManager(const PlatformTileManager&) = delete;
	PlatformTileManager& operator=(const PlatformTileManager&) = delete;

	PlatformTileManager(PlatformTileManager&&) = delete;
	PlatformTileManager& operator=(PlatformTileManager&&) = delete;

public:
	void Init(Renderer& renderer, std::uint32_t seed = 0xC0FFEEu);
	void Update(float cameraCenterY);
	void FindCollisions(SphereCollider& sphere, std::vector<CollisionManifold>& manifolds);
	void Render(IRenderer& renderer);
	void Reset();

private:
	struct TileCoordinate
	{
		int Column{};
		int Row{};

		bool operator==(const TileCoordinate& other) const
		{
			return Column == other.Column && Row == other.Row;
		}
	};

	struct TileCoordinateHash
	{
		std::size_t operator()(const TileCoordinate& coordinate) const;
	};

	struct ActivePolygon
	{
		TileCoordinate Coordinate{};
		std::size_t ResourceIndex{};
		std::unique_ptr<PPolygon> Polygon{};
	};

	struct PolygonImageResource
	{
		ID3D11Buffer* VertexBuffer{};
		ID3D11ShaderResourceView* ShaderResourceView{};
		UINT NumVertices{};
		std::vector<Vector3> Positions{};
	};

	void LoadPolygonImages(Renderer& renderer);
	void ReleasePolygonImages();
	void CreateTileInstance(const TileCoordinate& coordinate);
	float CalculateMaximumScale(std::size_t pointCount) const;
	std::uint32_t CreateRandomValue(const TileCoordinate& coordinate, std::size_t pointIndex, std::uint32_t salt) const;
	float CreateRandomFloat(const TileCoordinate& coordinate, std::size_t pointIndex, std::uint32_t salt, float minimum, float maximum) const;

private:
	std::vector<ActivePolygon> mPolygons{};
	std::unordered_set<TileCoordinate, TileCoordinateHash> mActiveTiles{};
	std::vector<PolygonImageResource> mPolygonImages{};

	std::uint32_t mSeed = 0xC0FFEEu;
	float mMinimumPolygonScale = 1.0f;
	Vector3 mPolygonLocalHalfExtents{ 1.0f, 1.0f, 0.0f };
	int mLastMinRow = 1;
	int mLastMaxRow = 0;
};
