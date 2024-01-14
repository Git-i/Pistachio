#pragma once
#define PT_CUSTOM_ENTRY
#include "Pistachio.h"
#include "ManagedBase.h"

namespace PistachioCS
{
	public ref class Asset : ManagedBase<Pistachio::Asset>
	{
	internal:
		Asset(Pistachio::Asset* ptr) { m_ptr = ptr; }
	public:
		Asset() {}
	};
}
