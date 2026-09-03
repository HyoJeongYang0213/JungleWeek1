#include "Polygon.h"
#include <algorithm>
#include <limits>

#include "../ImGui/stb_image.h"
#include "../Utils/Rnd.hpp"

namespace
{
	Vector3 NormalizeImagePoint(float x, float y, int width, int height)
	{
		const float aspectScaleX = width >= height ? 1.0f : static_cast<float>(width) / static_cast<float>(height);
		const float aspectScaleY = height >= width ? 1.0f : static_cast<float>(height) / static_cast<float>(width);

		return
		{
			(x / static_cast<float>(width) * 2.0f - 1.0f) * aspectScaleX,
			(1.0f - y / static_cast<float>(height) * 2.0f) * aspectScaleY,
			0.0f
		};
	}

	std::vector<Vector3> CreateConvexHull(std::vector<Vector3> points)
	{
		std::sort(points.begin(), points.end(), [](const Vector3& left, const Vector3& right)
			{
				return left.x == right.x ? left.y < right.y : left.x < right.x;
			});

		points.erase(std::unique(points.begin(), points.end(), [](const Vector3& left, const Vector3& right)
			{
				return left.x == right.x && left.y == right.y;
			}), points.end());

		if (points.size() < 3)
		{
			return {};
		}

		std::vector<Vector3> hull{};
		hull.reserve(points.size() * 2);

		for (const Vector3& point : points)
		{
			while (hull.size() >= 2)
			{
				const Vector3 edge = hull.back() - hull[hull.size() - 2];
				const Vector3 toPoint = point - hull[hull.size() - 2];
				if (edge.Cross(toPoint) > 0.0f)
				{
					break;
				}

				hull.pop_back();
			}

			hull.push_back(point);
		}

		const size_t lowerHullSize = hull.size();
		for (size_t index = points.size() - 1; index-- > 0;)
		{
			const Vector3& point = points[index];
			while (hull.size() > lowerHullSize)
			{
				const Vector3 edge = hull.back() - hull[hull.size() - 2];
				const Vector3 toPoint = point - hull[hull.size() - 2];
				if (edge.Cross(toPoint) > 0.0f)
				{
					break;
				}

				hull.pop_back();
			}

			hull.push_back(point);
		}

		hull.pop_back();
		return hull;
	}

	VertexTexture CreateTextureVertex(const Vector3& position, int width, int height)
	{
		const float aspectScaleX = width >= height ? 1.0f : static_cast<float>(width) / static_cast<float>(height);
		const float aspectScaleY = height >= width ? 1.0f : static_cast<float>(height) / static_cast<float>(width);

		return
		{
			position.x,
			position.y,
			position.z,
			(position.x / aspectScaleX + 1.0f) * 0.5f,
			(1.0f - position.y / aspectScaleY) * 0.5f
		};
	}

	std::vector<Vector3> ScaleVertices(const std::vector<Vector3>& vertices, const Vector3& halfExtents)
	{
		std::vector<Vector3> scaledVertices{};
		scaledVertices.reserve(vertices.size());

		for (const Vector3& vertex : vertices)
		{
			scaledVertices.emplace_back(vertex.x * halfExtents.x, vertex.y * halfExtents.y, vertex.z);
		}

		return scaledVertices;
	}
}

PolygonImageData PPolygon::CreateGeometryFromImage(const std::string& imagePath, unsigned char alphaThreshold)
{
	PolygonImageData result{};

	int width{};
	int height{};
	unsigned char* pixels = stbi_load(imagePath.c_str(), &width, &height, nullptr, STBI_rgb_alpha);
	if (pixels == nullptr || width <= 0 || height <= 0)
	{
		stbi_image_free(pixels);
		return result;
	}

	auto IsSolid = [pixels, width, height, alphaThreshold](int x, int y)
		{
			if (x < 0 || x >= width || y < 0 || y >= height)
			{
				return false;
			}

			return pixels[(y * width + x) * STBI_rgb_alpha + 3] >= alphaThreshold;
		};

	std::vector<Vector3> boundaryPoints{};
	boundaryPoints.reserve(static_cast<size_t>(width + height) * 4);

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if (!IsSolid(x, y))
			{
				continue;
			}

			if (IsSolid(x - 1, y) && IsSolid(x + 1, y) && IsSolid(x, y - 1) && IsSolid(x, y + 1))
			{
				continue;
			}

			boundaryPoints.emplace_back(NormalizeImagePoint(static_cast<float>(x), static_cast<float>(y), width, height));
			boundaryPoints.emplace_back(NormalizeImagePoint(static_cast<float>(x + 1), static_cast<float>(y), width, height));
			boundaryPoints.emplace_back(NormalizeImagePoint(static_cast<float>(x + 1), static_cast<float>(y + 1), width, height));
			boundaryPoints.emplace_back(NormalizeImagePoint(static_cast<float>(x), static_cast<float>(y + 1), width, height));
		}
	}

	stbi_image_free(pixels);
	result.Positions = CreateConvexHull(std::move(boundaryPoints));
	if (result.Positions.size() < 3)
	{
		result.Positions.clear();
		return result;
	}

	Vector3 center{};
	for (const Vector3& position : result.Positions)
	{
		center += position;
	}
	center /= static_cast<float>(result.Positions.size());

	result.Vertices.reserve(result.Positions.size() * 3);
	for (size_t index = 0; index < result.Positions.size(); ++index)
	{
		const Vector3& current = result.Positions[index];
		const Vector3& next = result.Positions[(index + 1) % result.Positions.size()];

		result.Vertices.emplace_back(CreateTextureVertex(center, width, height));
		result.Vertices.emplace_back(CreateTextureVertex(current, width, height));
		result.Vertices.emplace_back(CreateTextureVertex(next, width, height));
	}

	if (result.Vertices.size() > (std::numeric_limits<UINT>::max)())
	{
		return {};
	}

	result.NumVertices = static_cast<UINT>(result.Vertices.size());
	return result;
}

PPolygon::PPolygon(ID3D11Buffer* vb, UINT numVertices, const Vector3& position, const std::vector<Vector3>& vertices, bool isStatic, UINT vertexStride)
	: PPolygon{ vb, numVertices, position, Vector3{ 1.f, 1.f, 0.f }, vertices, isStatic, vertexStride }
{
}

PPolygon::PPolygon(ID3D11Buffer* vb, UINT numVertices, const Vector3& center, const Vector3& halfExtents, const std::vector<Vector3>& vertices, bool isStatic, UINT vertexStride)
	: mLocation{ center },
	mHalfExtents{ halfExtents },
	mVelocity{ 0.f, 0.f, 0.f },
	mMass{ Rnd::GetRandom(5.f, 15.f) },
	mRigidBody{ mLocation, mVelocity, mMass, 1.f },
	mCollider{ mRigidBody, ScaleVertices(vertices, mHalfExtents) },
	mVertexBuffer{ vb }, mNumVertices{ numVertices }, mVertexStride{ vertexStride }
{
	if (isStatic) {
		mMass = 0.f;
		mRigidBody.SetMomentOfInertia(0.f);
	}
	else {
		mRigidBody.SetMomentOfInertia(mCollider.CalculateMomentOfInertia(mMass));
	}
}

void PPolygon::Tick(float t)
{
	mRigidBody.Integrate(t);
}

ICollider& PPolygon::GetCollider()
{
	return mCollider;
}

RigidBody& PPolygon::GetRigidBody()
{
	return mRigidBody;
}

void PPolygon::Render(IRenderer& renderer)
{
	renderer.UpdateConstant(mLocation, mHalfExtents, mRigidBody.GetRotation());
	renderer.RenderPrimitive(mVertexBuffer, mNumVertices, mVertexStride);
}
