#pragma once
#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define EmbeddorOf(parent, member) \
    ((parent*) ((char*)this - offsetof(parent, member)))
#define PROPERTY(Class, parent_class,value_name, getter, setter)\
struct TOKENPASTE2(Property, __LINE__)__##Class : public Class\
{\
private:\
    Class get() getter;\
    void set(const Class& value)\
    {\
    parent_class* parent =  EmbeddorOf(parent_class, value_name);\
    Class& old_value = *this;\
    setter\
};\
public:\
    TOKENPASTE2(Property, __LINE__)__##Class(const Class& obj) {set(obj);}\
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
    operator Class() { return get(); }\
} value_name