#include "ptpch.h"
#include "MeshFactory.h"
namespace Pistachio
{
	Mesh* MeshFactory::CreatePlane()
	{
		std::vector<Vertex> positions = {
			{ /*positions*/-1, -1, 0, /*normals*/0, 0, -1, /*UV*/0, 1  },
			{ /*positions*/ 1, -1, 0, /*normals*/0, 0, -1, /*UV*/1, 1  },
			{ /*positions*/ 1,  1, 0, /*normals*/0, 0, -1, /*UV*/1, 0  },
			{ /*positions*/-1,  1, 0, /*normals*/0, 0, -1, /*UV*/0, 0  }
		};
		std::vector<unsigned int> indices = {
			2,1,0,0,3,2
		};
		return new Mesh(positions, indices);
	}
}