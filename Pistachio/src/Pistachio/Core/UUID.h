#pragma once
#include <xhash>
namespace Pistachio {
	class UUID 
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};
}
