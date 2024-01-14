#pragma once
#define PT_CUSTOM_ENTRY
#include "Pistachio.h"
#include "ManagedBase.h"

namespace PistachioCS
{
	public ref class UUID : ManagedBase<Pistachio::UUID>
	{
	public:
		bool operator==(const UUID% other) { return (*other.m_ptr) == (*m_ptr); }
		property System::UInt64^ Id
		{
			System::UInt64^ get()
			{
				return gcnew System::UInt64((m_ptr->operator size_t()));
			}
		}
	internal:
		UUID(uint64_t val);
	};
}
