#pragma once
namespace Pistachio {
	class RefCountedObject
	{
	public:
		int hold() const;
		int release() const;
		int count();
	private:
		mutable int m_count_ = 0;
	};
}
