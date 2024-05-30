#pragma once
#include "Pistachio/Core.h"
#include <bitset> //for hash
namespace Pistachio {
	class PISTACHIO_API UUID 
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		bool operator==(const UUID& other) const { return other.m_UUID == m_UUID; }
		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};
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
