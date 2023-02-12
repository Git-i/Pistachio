#include "ptpch.h"
#include "Physics.h"
physx::PxDefaultAllocator	   Pistachio::Physics::gAllocator     =   {};
Pistachio::ErrorCallBack       Pistachio::Physics::gErrorCallback =   {};
physx::PxFoundation*           Pistachio::Physics::gFoundation    = NULL;
physx::PxPhysics*              Pistachio::Physics::gPhysics       = NULL;
physx::PxDefaultCpuDispatcher* Pistachio::Physics::gDispatcher    = NULL;
physx::PxPvd*                  Pistachio::Physics::gPvd           = NULL;
physx::PxMaterial*             Pistachio::Physics::gMaterial      = NULL;
namespace Pistachio {
	void Physics::Init()
	{
		gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

		gPvd = PxCreatePvd(*gFoundation);
		physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("192.168.8.1", 5425, 10);
		gPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

		gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, physx::PxTolerancesScale(), true, gPvd);

	}
	void Physics::Shutdown()
	{
	}
}