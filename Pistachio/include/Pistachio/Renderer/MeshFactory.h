#pragma once
#include "Mesh.h"
namespace Pistachio {
	class MeshFactory
	{
	public:
		static Mesh* CreatePlane();
	};
};