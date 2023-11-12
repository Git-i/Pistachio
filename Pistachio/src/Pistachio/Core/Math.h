#pragma once
#include "SimpleMath.h"
namespace Pistachio
{
	using Vector2 = DirectX::SimpleMath::Vector2;
    using Vector3 = DirectX::SimpleMath::Vector3;
    using Vector4 = DirectX::SimpleMath::Vector4;
    using Matrix4 = DirectX::SimpleMath::Matrix;
    using Quaternion = DirectX::SimpleMath::Quaternion;
    using Plane = DirectX::SimpleMath::Plane;

    class Math
    {
    public:
        static float ToRadians(float AngleInDegrees) { return AngleInDegrees * (DirectX::XM_PI / 180.f); }
        static float ToDegrees(float AngleInRadians) { return AngleInRadians * (180.f * DirectX::XM_PI); }
    };
}