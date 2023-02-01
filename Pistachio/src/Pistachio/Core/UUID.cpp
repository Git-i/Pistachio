#include "ptpch.h"
#include "UUID.h"

#include <random>
namespace Pistachio {
	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;
	UUID::UUID()
		:m_UUID(s_UniformDistribution(s_Engine))
	{
	}
	UUID::UUID(uint64_t uuid)
	{
	}

}
namespace std {
	template<>
	struct hash<Pistachio::UUID>
	{
		std::size_t operator()(const Pistachio::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}