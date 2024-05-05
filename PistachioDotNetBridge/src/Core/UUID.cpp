#include "pch.h"
#include "UUID.h"

namespace PistachioCS
{
	UUID::UUID(uint64_t val) { m_ptr = new Pistachio::UUID(val); }
}
