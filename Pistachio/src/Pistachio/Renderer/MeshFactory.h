#pragma once
#include "Mesh.h"
namespace Pistachio {
	class PISTACHIO_API MeshFactory
	{
	public:
		//Generate A Plane
		static Mesh* CreatePlane();
		//Generate A Cube
		static Mesh* CreateCube();
		/*Generate a UV Shphere mesh with uneven vertex distribution:
		* slices : number of longitude lines
		* stacks : number of latitude lines */
		static Mesh* CreateUVSphere(int slices, int stacks);
		/*Generate an Ico Sphere from a subdived icosahedron with more even vertex distribution
		* resolution: number of subdivision*/
		static Mesh* CreateIcoSphere(int resolutions);
		/*Generate a sphere from a subdivided cube
		* resolution: number of subdivision*/
		static Mesh* CreateQuadSphere(int resolution);
		//Generate a (Tetrahedron/Hexahedron/Octahedron/Dodecahedron/Icosahedron)
		static Mesh* CreatePlatonic();
	};
};