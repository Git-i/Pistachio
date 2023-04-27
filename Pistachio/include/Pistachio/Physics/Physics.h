#pragma once
#include "PxPhysicsAPI.h"
#include "../Core/Log.h"
namespace Pistachio {
	class ErrorCallBack : public physx::PxErrorCallback
	{
		virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override
		{
			// error processing implementation
			PT_CORE_INFO("{0}", message);
		}
	};

	class Physics
	{
	public:
		static physx::PxDefaultAllocator	  gAllocator;
		static ErrorCallBack                  gErrorCallback;
		static physx::PxFoundation*           gFoundation;
		static physx::PxPhysics*              gPhysics;
		static physx::PxDefaultCpuDispatcher* gDispatcher;
		static physx::PxPvd*                  gPvd;
		static physx::PxMaterial*             gMaterial;
	public:
		static void Init();
		static void Shutdown();
	};
}