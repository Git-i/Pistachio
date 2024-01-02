#pragma once
#define PT_PLATFORM_WINDOWS 1
#define PISTACHIO_RENDER_API_DX11 1
typedef int KeyCode;
#define ENUM_FLAGS(EnumType)                      \
inline EnumType operator|(EnumType a, EnumType b) {                             \
    return static_cast<EnumType>(static_cast<int>(a) | static_cast<int>(b));    \
}                                                                               \
inline EnumType operator&(EnumType a, EnumType b) {                             \
    return static_cast<EnumType>(static_cast<int>(a) & static_cast<int>(b));    \
}                                                                               \
inline EnumType operator^(EnumType a, EnumType b) {                             \
    return static_cast<EnumType>(static_cast<int>(a) ^ static_cast<int>(b));    \
}                                                                               \
inline EnumType operator~(EnumType a) {                                         \
    return static_cast<EnumType>(~static_cast<int>(a));                         \
}                                                                               \
inline EnumType& operator|=(EnumType& a, EnumType b) {                         \
    a = a | b;                                                                 \
    return a;                                                                  \
}                                                                               \
inline EnumType& operator&=(EnumType& a, EnumType b) {                         \
    a = a & b;                                                                 \
    return a;                                                                  \
}                                                                               \
inline EnumType& operator^=(EnumType& a, EnumType b) {                         \
    a = a ^ b;                                                                 \
    return a;                                                                  \
}


#ifdef PT_PLATFORM_WINDOWS
#ifdef DYNAMICLINK
	#ifdef PISTACHIO_BUILD_DLL
		#define PISTACHIO_API __declspec(dllexport)
	#else
		#define PISTACHIO_API __declspec(dllimport)
	#endif
#else
	#define PISTACHIO_API
#endif
#else
	#error Pistachio Engine Only Supports Windows
#endif // PT_PLATFROM_WINDOWS
#if defined _WIN64
//2411 lmao
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#if !(defined(PISTACHIO_RENDER_API_DX11) || defined(PISTACHIO_RENDER_API_VULKAN) || defined(PISTACHIO_RENDER_API_DX12))
#define STRING2(x) #x
#define STRING(x) STRING2(x)
#pragma message(__FILE__ "(" STRING(__LINE__) "): warning: No Render API was selected, defaulting to DX11")
#define PISTACHIO_RENDER_API_DX11
#endif // !PISTACHIO_RENDER_API_DX11
#if defined(PISTACHIO_RENDER_API_VULKAN)
#define STRING2(x) #x
#define STRING(x) STRING2(x)
#pragma message(__FILE__ "(" STRING(__LINE__) "): warning: Vulkan is Currently not supported, defaulting to DX11")
#define PISTACHIO_RENDER_API_DX11
#endif // PISTACHIO_RENDER_API_VULKAN
#if defined(PISTACHIO_RENDER_API_DX12)
#define STRING2(x) #x
#define STRING(x) STRING2(x)
#pragma message(__FILE__ "(" STRING(__LINE__) "): warning: DirectX 12 is Currently not supported, defaulting to DX11")
#define PISTACHIO_RENDER_API_DX11
#endif // PISTACHIO_RENDER_API_DX12
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

namespace Pistachio {
	template <typename T>
	using Scope =  std::unique_ptr<T>;
	template <typename T>
	using Ref = std::shared_ptr<T>;
	enum class Byte : std::uint8_t
	{
		Empty = 0,
		Bit1 = 1 << 0,
		Bit2 = 1 << 1,
		Bit3 = 1 << 2,
		Bit4 = 1 << 3,
		Bit5 = 1 << 4,
		Bit6 = 1 << 5,
		Bit7 = 1 << 6,
		Bit8 = 1 << 7,
	};
	ENUM_FLAGS(Byte);
	
}
#ifdef _DEBUG
#define PT_CORE_ASSERT(x) if(x){}else{__debugbreak();}
#else
#define PT_CORE_ASSERT(x) x
#endif // _DEBUG
#pragma comment(lib, "XInput.lib")
#pragma comment(lib, "Xinput9_1_0.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "Comdlg32.lib")

#include<wrl.h>
