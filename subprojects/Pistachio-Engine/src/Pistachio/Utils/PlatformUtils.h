#pragma once
#include <string>
#include <type_traits>
#include <algorithm>
namespace Pistachio {
	class PISTACHIO_API FileDialogs {
	public:
		static std::string SaveFile(const char* filter);
		static std::string OpenFile(const char* filter);
	};
	enum Edianness
	{
		Big, Little
	};
	class PISTACHIO_API Edian
	{
		template<typename T>
		static T byteswap(T value)
		{
			static_assert(std::is_integral_v<T>, "T must be an integral type");
			static_assert(std::has_unique_object_representations_v<T>, "T may not have padding bits");
			std::array<uint8_t, sizeof(T)> value_representation;
			for (int i = 0; i < sizeof(T); i++)
			{
				memcpy(value_representation.data() + i, (uint8_t*)(&value) + i, 1);
			}
			std::reverse(value_representation.begin(), value_representation.end());
			T retVal{};
			memcpy(&retVal, value_representation.data(), sizeof(T));
			return retVal;
		}
	public:
		static Edianness GetEdianness()
		{
			// Fast way to retrieve edianness of current system
			int i = 1;
			return *(char*)&i == 1 ? Edianness::Little : Edianness::Big;
		}
		template<typename T>
		static T ConvertToBigEndian(T value)
		{
			if (GetEdianness() == Edianness::Big) return value;
			return byteswap<T>(value);
		}
		template<typename T>
		static T ConvertToLittleEndian(T value)
		{
			if (GetEdianness() == Edianness::Little) return value;
			return byteswap<T>(value);
		}
		template<typename T>
		static T ConvertToSystemEndian(T value, Edianness inputEdianness)
		{
			if (GetEdianness() == inputEdianness) return value;
			return byteswap<T>(value);
		}
	};
}
