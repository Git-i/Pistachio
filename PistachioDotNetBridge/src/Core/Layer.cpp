#include "pch.h"
#include "Layer.h"

namespace PistachioCS
{
	Layer::Layer()
	{
		m_ptr = new LayerWrapper(this);
	}
}
