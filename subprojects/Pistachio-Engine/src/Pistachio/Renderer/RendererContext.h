#pragma once
/*
Holds data for the Renderer

*/
#include "Pistachio/Core/Math.h"
#include "FormatsAndTypes.h"
#include "Pistachio/Core.h"
#include "Pistachio/Allocators/FreeList.h"
#include "Core/Device.h"
#include "Pistachio/Renderer/Buffer.h"
#include "Pistachio/Renderer/BufferHandles.h"
#include "Pistachio/Renderer/RendererBase.h"
#include "Pistachio/Renderer/Shader.h"
#include "Pistachio/Renderer/Texture.h"
#include <cstdint>
#include <functional>
#include <zconf.h>
namespace Pistachio
{
	struct PISTACHIO_API TransformData
	{
		Matrix4 transform;
		Matrix4 normal;
	};
    struct MonolithicBufferAllocator
    {
        void Initialize(uint32_t initialSize);
        uint32_t     freeFastSpace;
		uint32_t     freeSpace;    
		uint32_t     capacity;
		FreeList     freeList;
        /*
		 * handles map buffer handles to thier actual offsets, in case defragmentation moves them around
		 * each handle is just an offset into this vector
		 */
        std::vector<uint32_t> HandleOffsets;
		std::vector<uint32_t> UnusedHandles;
		uint32_t AssignHandle(std::uint32_t offset);
		RendererVBHandle Allocate(
			const std::function<void(uint32_t)>&, const std::function<void()>&,
			uint32_t size,
			RHI::Ptr<RHI::Buffer> buffer = nullptr, 
			const void* initialData = nullptr);
		void DeAllocate(RendererVBHandle handle);	
    };
    struct MonolithicBuffer
    {
        void Initialize(uint32_t initialSize, RHI::BufferUsage usage);
        RHI::Ptr<RHI::Buffer> buffer;
        MonolithicBufferAllocator allocator;
    };
    struct FrameResource
	{
        void Initialize(uint32_t cbCapacity);
		ConstantBuffer transformBuffer;
		RHI::Ptr<RHI::DynamicDescriptor> transformBufferDesc;
		RHI::Ptr<RHI::DynamicDescriptor> transformBufferDescPS;
	};
    class PISTACHIO_API RendererContext
    {
    public:
		void Initailize();
		MonolithicBuffer meshVertices;
        MonolithicBuffer meshIndices;
		/*
		 * Constant Buffers are a little bit tricky
		 * Handling updates to all buffers is the job of the caller
		 * but handling allocations and defragments is the job of the renderer
		 * An option would be to keep a uint for every frame that needs an op
		 * eg uint32_t numDirtyAllocs; OnUpdate(){if(numDirtyAllocs){}}
		 * but then we'd have to store op's order and details.
		 * We'd probably wait for the gpu to get to the current frame, before doing any op's except updating
		 * since updating is handled by the scene, it'll have the data needed to recreate the update event
		 * 
		 * Im leaving the above comment, but i might just have found a better way.
		 * Allocate wouldn't update just give you a handle, and you update for all three buffers
		 * Deallocate would just update the free list
		 * Grow and Defrag would however block
		 */
		
		MonolithicBufferAllocator constantBufferAllocator;
		uint32_t     numDirtyCBFrames;
		FrameResource resources[RendererBase::numFramesInFlight];
		Texture2D BrdfTex;

		SamplerHandle defaultSampler;
		SamplerHandle brdfSampler;
		SamplerHandle shadowSampler;

		StructuredBuffer computeShaderMiscBuffer;
		std::unordered_map<std::string, ComputeShader*> computeShaders;
		std::unordered_map<std::string, Shader*> shaders;
    };
    
}