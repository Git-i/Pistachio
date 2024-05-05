#include "ptpch.h"
#include "CullingManager.h"

bool Pistachio::CullingManager::FrustumCull(const BoundingBox& aabb, const BoundingFrustum& frustum)
{
    if (frustum.Intersects(aabb) || frustum.Contains(aabb))
    {
        return true;
    }
    return false;
}

bool Pistachio::CullingManager::FrustumCull(const BoundingSphere& sphere, const BoundingFrustum& frustum)
{
    if (frustum.Intersects(sphere) || frustum.Contains(sphere))
    {
        return true;
    }
    return false;
}

bool Pistachio::CullingManager::SphereCull(const BoundingBox& aabb, const BoundingSphere& sphere)
{
    if (sphere.Intersects(aabb) || sphere.Contains(aabb))
    {
        return true;
    }
    return false;
}
