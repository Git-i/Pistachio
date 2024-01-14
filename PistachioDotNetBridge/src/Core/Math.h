#pragma once
#define PT_CUSTOM_ENTRY
#include "Pistachio.h"
#include "ManagedBase.h"

namespace PistachioCS
{
	public ref class Vector3
	{
	public:
		float x, y, z;
        Vector3(): x(0),y(0),z(0) {}
        Vector3(float ix) :x(ix), y(ix), z(ix) {}
        Vector3(float ix, float iy, float iz) :x(ix), y(iy), z(iz) {}
        Vector3(const Vector3% other) { x = other.x, y = other.y, z = other.z; }
        Vector3% operator=(const Vector3% other) { x = other.x, y = other.y, z = other.z; return *this; }
        // Comparison operators
        bool operator == (const Vector3% V) { return (x == V.x && y == V.y && z == V.z); };
        bool operator != (const Vector3% V) { return !((*this) == V); };

        Vector3% operator+= (const Vector3% V) { x += V.x, y += V.y, z += V.z; return *this; };
        Vector3% operator-= (const Vector3% V) {return *this;};
        Vector3% operator*= (const Vector3% V) {return *this;};
        Vector3% operator*= (float S) {return *this;};
        Vector3% operator/= (float S) {return *this;};
	};
}
