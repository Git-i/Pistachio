#pragma once

namespace PistachioCS
{
	template<typename T>
	public ref class ManagedBase
	{
	internal:
		T* m_ptr = nullptr;
	public:
		~ManagedBase()
		{
			this->!ManagedBase();
		}
		!ManagedBase()
		{
			if (m_ptr)
			{
				delete m_ptr;
				m_ptr = nullptr;
			}
		}
		
	};
}
