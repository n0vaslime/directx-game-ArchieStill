#pragma once

#include <DirectXCollision.h>
#include <array>
#include <limits>
#include <algorithm>

#include "CMOGO.h"

using namespace DirectX;

namespace Collision
{
	namespace OBB
	{
		struct MinMax
		{
			float min;
			float max;
		};

		void getOBBFaceNormals(const BoundingOrientedBox& worldspace_box, std::array<XMFLOAT3, 8>& corners_world, XMVECTOR* out_axes)
		{
			//std::array<XMFLOAT3, 8> corners_world;

			worldspace_box.GetCorners(&corners_world[0]);

			const auto origin	= XMLoadFloat3(&corners_world[4]);
			const auto x_dir	= XMLoadFloat3(&corners_world[5]) - origin;
			const auto y_dir	= XMLoadFloat3(&corners_world[7]) - origin;
			const auto z_dir	= XMLoadFloat3(&corners_world[0]) - origin;

			out_axes[0]			= XMVector3Normalize(XMVector3Cross(x_dir, y_dir));
			out_axes[1]			= XMVector3Normalize(XMVector3Cross(y_dir, z_dir));
			out_axes[2]			= XMVector3Normalize(XMVector3Cross(z_dir, x_dir));
		}


		MinMax projectBoxOnAxes(std::array<XMFLOAT3, 8>& corners, const XMVECTOR& ax)
		{
			// Find projection on axes
			float min = std::numeric_limits<float>::infinity();
			float max = -std::numeric_limits<float>::infinity();

			for (const auto& vertex : corners)
			{
				float projection;
				XMStoreFloat(&projection, XMVector3Dot(XMLoadFloat3(&vertex), ax));
				if (projection < min) min = projection;
				if (projection > max) max = projection;
			}

			return MinMax{ min, max };
		}
	}

	XMFLOAT3 ejectionCMOGO(CMOGO& go1, CMOGO& go2)
	{
		// Create & Populate Collision Axes
		std::array<XMVECTOR, 6 > collision_axes;

		BoundingOrientedBox worldbox_1, worldbox_2;
		std::array<XMFLOAT3, 8> corners_world_1, corners_world_2;

		go1.getCollider().Transform(worldbox_1, go1.getWorldTransform());
		OBB::getOBBFaceNormals(worldbox_1, corners_world_1, &collision_axes[0]);

		go2.getCollider().Transform(worldbox_2, go2.getWorldTransform());
		OBB::getOBBFaceNormals(worldbox_2, corners_world_2, &collision_axes[3]);


		float pen_dist = numeric_limits<float>::infinity();
		XMVECTOR pen_vector;

		for (const auto& ax : collision_axes)
		{
			// Get Min and Max overlaps of box vertixes on axes
			const auto minmax_1 = OBB::projectBoxOnAxes(corners_world_1, ax);
			const auto minmax_2 = OBB::projectBoxOnAxes(corners_world_2, ax);

			// Calculate collision depth
			float overlap = min(minmax_1.max, minmax_2.max) - max(minmax_1.min, minmax_2.min);

			// Check if this overlap is the smallest ejection thus far
			if (overlap < pen_dist)
			{
				pen_dist = overlap;
				pen_vector = ax;

				XMFLOAT3 v;
				XMStoreFloat3(&v, ax);
			}
		}

		XMFLOAT3 out;
		XMStoreFloat3(&out, pen_vector);

		auto diff = worldbox_1.Center - worldbox_2.Center;
		float orientation;
		XMStoreFloat(&orientation, XMVector3Dot(XMLoadFloat3(&diff), pen_vector));

		if (orientation > 0) pen_dist *= -1;

		return out * pen_dist;
	}
}