#pragma once
#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define EmbeddorOf(parent, member) \
    ((parent*) ((char*)this - offsetof(parent, member)))
#define PROPERTY(Class, parent_class,value_name, getter, setter)\
struct TOKENPASTE2(Property, __LINE__)__##Class##value_name\
{\
private:\
    Class this_value;\
    Class get() const getter;\
    void set(const Class& value)\
    {\
    parent_class* parent =  EmbeddorOf(parent_class, value_name);\
    Class& old_value = this->this_value;\
    setter\
};\
public:\
    TOKENPASTE2(Property, __LINE__)__##Class##value_name (){}\
    TOKENPASTE2(Property, __LINE__)__##Class##value_name(const Class& obj) {set(obj);}\
    template<typename T, typename _ = typename std::enable_if<(sizeof(std::declval<Class&>() += std::declval<T>()) > 2)>::type>\
        auto operator+=(T a) { auto _temp = get(); set(_temp += a); };\
    template<typename T, typename _ = typename std::enable_if<(sizeof(std::declval<Class&>() -= std::declval<T>()) > 2)>::type>\
        auto operator-=(T a) { auto _temp = get(); set(_temp -= a); };\
    template<typename T, typename _ = typename std::enable_if<(sizeof(std::declval<Class&>() *= std::declval<T>()) > 2)>::type>\
        auto operator*=(T a) { auto _temp = get(); set(_temp *= a); };\
    template<typename T, typename _ = typename std::enable_if<(sizeof(std::declval<Class&>() /= std::declval<T>()) > 2)>::type>\
        auto operator/=(T a) { auto _temp = get(); set(_temp /= a); };\
    template<typename T, typename _ = typename std::enable_if<(sizeof(std::declval<Class&>() %= std::declval<T>()) > 2)>::type>\
        auto operator%=(T a) { auto _temp = get(); set(_temp %= a); };\
    template<typename T, typename _ = typename std::enable_if<(sizeof(std::declval<Class&>() &= std::declval<T>()) > 2)>::type>\
        auto operator&=(T a) { auto _temp = get(); set(_temp &= a); };\
    template<typename T, typename _ = typename std::enable_if<(sizeof(std::declval<Class&>() |= std::declval<T>()) > 2)>::type>\
        auto operator|=(T a) { auto _temp = get(); set(_temp |= a); };\
    template<typename T, typename _ = typename std::enable_if<(sizeof(std::declval<Class&>() ^= std::declval<T>()) > 2)>::type>\
        auto operator^=(T a) { auto _temp = get(); set(_temp ^= a); };\
    template<typename T, typename _ = typename std::enable_if<(sizeof(std::declval<Class&>() += std::declval<T>()) > 2)>::type>\
        auto operator<<=(T a) { auto _temp = get(); set(_temp <<= a); };\
    template<typename T, typename _ = typename std::enable_if<(sizeof(std::declval<Class&>() += std::declval<T>()) > 2)>::type>\
        auto operator>>=(T a) { auto _temp = get(); set(_temp >>= a); };\
    void operator=(Class a) { set(a); };\
    operator Class() const { return get(); }\
} value_name

#define VEC3(pclass, obj, ex_notify)\
struct TOKENPASTE2(Vec3,__LINE__)\
{\
	void notify()\
	{\
		auto* parent = EmbeddorOf(pclass, obj);\
		parent->bDirty = true;\
	}\
	PROPERTY(float, TOKENPASTE2(Vec3,__LINE__), x, { return this_value; }, { old_value = value; parent->notify(); });\
	PROPERTY(float, TOKENPASTE2(Vec3,__LINE__), y, { return this_value; }, { old_value = value; parent->notify(); });\
	PROPERTY(float, TOKENPASTE2(Vec3,__LINE__), z, { return this_value; }, { old_value = value; parent->notify(); });\
	operator Vector3() const { return Vector3(x, y, z); }\
	operator DirectX::XMVECTOR() const { Vector3 val = (*this);  return DirectX::XMLoadFloat3(&val); }\
	const TOKENPASTE2(Vec3,__LINE__)& operator=(const Vector3& other) { this->x = other.x, this->y = other.y, this->z = other.z; return *this; }\
	const TOKENPASTE2(Vec3,__LINE__)& operator=(const DirectX::XMVECTOR& other) { Vector3 val = other; return (*this) = val; }\
} obj
#define QUAT(pclass, obj, ex_notify)\
struct TOKENPASTE2(Quat,__LINE__)\
{\
	void notify()\
	{\
		auto* parent = EmbeddorOf(pclass, obj);\
		parent->bDirty = true;\
        ex_notify;\
	}\
	PROPERTY(float, TOKENPASTE2(Quat,__LINE__), x, { return this_value; }, { old_value = value; parent->notify(); });\
	PROPERTY(float, TOKENPASTE2(Quat,__LINE__), y, { return this_value; }, { old_value = value; parent->notify(); });\
	PROPERTY(float, TOKENPASTE2(Quat,__LINE__), z, { return this_value; }, { old_value = value; parent->notify(); });\
	PROPERTY(float, TOKENPASTE2(Quat,__LINE__), w, { return this_value; }, { old_value = value; parent->notify(); });\
	operator Quaternion() const { return Quaternion(x, y, z, w); }\
	operator DirectX::XMVECTOR() const { Quaternion val = (*this);  return DirectX::XMLoadFloat4(&val); }\
	const TOKENPASTE2(Quat,__LINE__)& operator=(const Quaternion& other) { this->x = other.x, this->y = other.y, this->z = other.z, this->w = other.w; return *this; }\
	const TOKENPASTE2(Quat,__LINE__)& operator=(const DirectX::XMVECTOR& other) { Quaternion val = other; return (*this) = val; }\
}obj
