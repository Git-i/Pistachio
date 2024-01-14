#pragma once
#include "Pistachio\Core\Math.h"
namespace Pistachio
{
	class PISTACHIO_API CullingManager
	{
	public:
		static bool FrustumCull(const BoundingBox& aabb, const BoundingFrustum& frustum );    //returns true if the object is visible
		static bool FrustumCull(const BoundingSphere& sphere, const BoundingFrustum& frustum ); //returns true if the object is visible
		static bool SphereCull(const BoundingBox& aabb, const BoundingSphere& sphere);		  //returns true if the object is visible
	};
}
