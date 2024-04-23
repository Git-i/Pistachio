#pragma once
#include "Pistachio\Core.h"
#include "SimpleMath.h"
namespace Pistachio
{
	using Vector2 = DirectX::SimpleMath::Vector2;
    using Vector3 = DirectX::SimpleMath::Vector3;
    using Vector4 = DirectX::SimpleMath::Vector4;
    using Matrix4 = DirectX::SimpleMath::Matrix;
    using Quaternion = DirectX::SimpleMath::Quaternion;
    using Plane = DirectX::SimpleMath::Plane;
    using BoundingBox = DirectX::BoundingBox;
    using BoundingFrustum = DirectX::BoundingFrustum;
    using BoundingSphere = DirectX::BoundingSphere;

    struct iVector2
    {
        iVector2() : x(0), y(0) {};
        iVector2(std::uint32_t _x, std::uint32_t _y) : x(_x), y(_y) {};
        std::uint32_t x, y;
        iVector2 operator+(const iVector2& other)
        {
            return { x + other.x, y + other.y };
        }
        iVector2 operator-(const iVector2& other)
        {
            return { x - other.x, y - other.y };
        }
        iVector2 operator*(const iVector2& other)
        {
            return { x * other.x, y * other.y };
        }
        iVector2 operator/(const iVector2& other)
        {
            return { x / other.x, y / other.y };
        }
        void operator+=(const iVector2& other)
        {
            x = x + other.x;
            y = y + other.y;
        }
        void operator-=(const iVector2& other)
        {
            x = x - other.x;
            y = y - other.y;
        }
        void operator*=(const iVector2& other)
        {
            x = x * other.x;
            y = y * other.y;
        }
        void operator/=(const iVector2& other)
        {
            x = x / other.x;
            y = y / other.y;
        }
    };
    struct hiVector2
    {
        std::uint16_t x, y;
        hiVector2(iVector2 vec) : x(vec.x), y(vec.y) {}
        hiVector2() : x(0), y(0) {}
    };
    class Math
    {
    public:
        static float ToRadians(float AngleInDegrees) { return AngleInDegrees * (DirectX::XM_PI / 180.f); }
        static float ToDegrees(float AngleInRadians) { return AngleInRadians * (180.f / DirectX::XM_PI); }
    };
}