#pragma once
#define PT_CUSTOM_ENTRY
#include "Pistachio.h"
#include "ManagedBase.h"

namespace PistachioCS
{
	public ref class Vector3
	{
    internal:
        float _x,_y,_z;
	public:
		property float x
        {
            float get(){return _x;}
            void set(float value) { _x = value; }
        }
        property float y
        {
            float get() { return _y; }
            void set(float value) { _y = value; }
        }
        property float z
        {
            float get() { return _z; }
            void set(float value) { _z = value; }
        }
        Vector3(): _x(0),_y(0),_z(0) {}
        Vector3(float i_x) :_x(i_x), _y(i_x), _z(i_x) {}
        Vector3(float i_x, float i_y, float i_z) :_x(i_x), _y(i_y), _z(i_z) {}
        Vector3(const Vector3% other) { _x = other._x, _y = other._y, _z = other._z; }
        Vector3% operator=(const Vector3% other) { _x = other._x, _y = other._y, _z = other._z; return *this; }
        // Comparison operators
        bool operator == (const Vector3% V) { return (_x == V._x && _y == V._y && _z == V._z); };
        bool operator != (const Vector3% V) { return !((*this) == V); };

        Vector3% operator+= (const Vector3% V) { _x += V._x, _y += V._y, _z += V._z; return *this; };
        Vector3% operator-= (const Vector3% V) {return *this;};
        Vector3% operator*= (const Vector3% V) {return *this;};
        Vector3% operator*= (float S) {return *this;};
        Vector3% operator/= (float S) {return *this;};
	};
}
