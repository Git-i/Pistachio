#pragma once
#define PT_CUSTOM_ENTRY
#include "Pistachio.h"
#include "ManagedBase.h"

namespace PistachioCS
{
	public ref class Asset : ManagedBase<Pistachio::Asset>
	{
	public:
		Asset() { m_ptr = new Pistachio::Asset(); }

	};
}
