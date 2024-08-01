#include "Pistachio/Renderer/RenderGraph.h"
#include "Pistachio/Renderer/RendererBase.h"
#include "Pistachio/Renderer/Shader.h"
#include "Texture.h"
#include "ptpch.h"
#include "Renderer.h"
#include "Material.h"
#include "../Scene/Scene.h"
#include "Pistachio/Core/Window.h"
#include "Pistachio/Core/Math.h"
static const uint32_t VB_INITIAL_SIZE = 1024;
static const uint32_t IB_INITIAL_SIZE = 1024;
static const uint32_t INITIAL_NUM_LIGHTS = 20;
static const uint32_t INITIAL_NUM_OBJECTS = 20;

static const uint32_t NUM_SKYBOX_MIPS = 5;

namespace Pistachio {
	void Renderer::Init(const char* skyboxFile)
	{
	}
	void Renderer::ChangeRGTexture(RGTextureHandle& texture, RHI::ResourceLayout newLayout, RHI::ResourceAcessFlags newAccess,RHI::QueueFamily newFamily)
	{
		texture.originVector->at(texture.offset).current_layout = newLayout;
		texture.originVector->at(texture.offset).currentAccess = newAccess;
		texture.originVector->at(texture.offset).currentFamily = newFamily;
	}
	
	void Renderer::EndScene()
	{
		PT_PROFILE_FUNCTION()
		RendererBase::EndFrame();
	}
	void Renderer::Shutdown() {
		
		RendererBase::Shutdown();
	}
	const RendererVBHandle Renderer::AllocateVertexBuffer(uint32_t size,const void* initialData)
	{
		return AllocateBuffer(vbFreeList, vbFreeSpace, vbFreeFastSpace, vbCapacity, &Renderer::GrowMeshVertexBuffer,&Renderer::DefragmentMeshVertexBuffer, vbHandleOffsets,vbUnusedHandles,&meshVertices, size, initialData);
	}
	const RendererIBHandle Renderer::AllocateIndexBuffer(uint32_t size, const void* initialData)
	{
		auto [a,b] = AllocateBuffer(ibFreeList, ibFreeSpace, ibFreeFastSpace, ibCapacity, &Renderer::GrowMeshIndexBuffer, &Renderer::DefragmentMeshIndexBuffer,ibHandleOffsets,ibUnusedHandles,&meshIndices, size, initialData);
		return { a,b };
	}
	ComputeShader* Renderer::GetBuiltinComputeShader(const std::string& name)
	{
		if (auto it = computeShaders.find(name); it != computeShaders.end())
		{
			return it->second;
		}
		return nullptr;
	}
	Shader* Renderer::GetBuiltinShader(const std::string& name)
	{
		if (auto it = shaders.find(name); it != shaders.end())
		{
			return it->second;
		}
		return nullptr;
	}
	const RendererCBHandle Renderer::AllocateConstantBuffer(uint32_t size)
	{
		auto [a,b] = AllocateBuffer(cbFreeList, cbFreeSpace, cbFreeFastSpace, cbCapacity, &Renderer::GrowConstantBuffer, &Renderer::DefragmentConstantBuffer, cbHandleOffsets, cbUnusedHandles, nullptr, RendererUtils::ConstantBufferElementSize(size), nullptr);
		return { a,b };
	}
	RenderCubeMap& Pistachio::Renderer::GetSkybox()
	{
		return skybox;
	}
	inline RendererVBHandle Renderer::AllocateBuffer(
		FreeList& flist, uint32_t& free_space, 
		uint32_t& fast_space, 
		uint32_t& capacity,
		decltype(&Renderer::GrowMeshVertexBuffer) grow_fn,
		decltype(&Renderer::DefragmentMeshVertexBuffer) defrag_fn,
		std::vector<uint32_t>& offsetsVector,
		std::vector<uint32_t>& freeHandlesVector,
		RHI::Ptr<RHI::Buffer>* buffer, 
		uint32_t size,
		const void* initialData)
	{
		RendererVBHandle handle;
		//check if we have immediate space available
		if (size < fast_space)
		{
			//allocate to buffer end
			//fast space is always at the end
			PT_CORE_ASSERT(flist.Allocate(capacity - fast_space, size) == 0);
			if (initialData)
			{
				RendererBase::PushBufferUpdate(*buffer, capacity - fast_space, initialData, size);
				RendererBase::FlushStagingBuffer();
			}
			handle.handle = AssignHandle(offsetsVector,freeHandlesVector,capacity - fast_space);
			handle.size = size;
			fast_space -= size;
			free_space -= size;
			return handle;
		}
		//if not, check if we have space at all
		else if (size < free_space)
		{
			//if we have space, check the free list to see if space is continuos
			if (auto space = flist.Find(size); space != UINT32_MAX)
			{
				//allocate
				PT_CORE_ASSERT(flist.Allocate(space, size) == 0);
				if (initialData)
				{
					RendererBase::PushBufferUpdate(*buffer, space, initialData, size);
					RendererBase::FlushStagingBuffer();
				}
				handle.handle = AssignHandle(offsetsVector, freeHandlesVector, space);
				handle.size = size;
				free_space -= size;
				return handle;
			}
			PT_CORE_WARN("Defragmenting Buffer");
			defrag_fn();
			if (initialData)
			{
				RendererBase::PushBufferUpdate(*buffer, capacity - fast_space, initialData, size);
				RendererBase::FlushStagingBuffer();
			}
			handle.handle = AssignHandle(offsetsVector,freeHandlesVector,capacity - fast_space);
			handle.size = size;
			fast_space -= size;
			free_space -= size;
			return handle;
			//allocate to buffer end
		}
		else
		{
			PT_CORE_WARN("Growing Buffer");
			grow_fn(size);
			//growth doesnt guarantee that the free space is "Fast" it just guarantees we'll have enough space for the op
			return AllocateBuffer(flist, free_space, fast_space, capacity,grow_fn,defrag_fn,offsetsVector,freeHandlesVector,buffer,size, initialData);
			//allocate to buffer end
		}
		return handle;
	}
	uint32_t Renderer::AssignHandle(std::vector<uint32_t>& offsetsVector, std::vector<uint32_t>& freeHandlesVector, std::uint32_t offset)
	{
		if (freeHandlesVector.empty())
		{
			offsetsVector.push_back(offset);
			return(uint32_t)( offsetsVector.size() - 1);
		}
		uint32_t handle = freeHandlesVector[freeHandlesVector.size()-1];
		offsetsVector[handle] = offset;
		freeHandlesVector.pop_back();
		return (uint32_t)handle;
	}
	void Renderer::GrowMeshVertexBuffer(uint32_t minSize)
	{
		const uint32_t GROW_FACTOR = 20; //probably use a better, more size dependent method to determing this
		uint32_t new_size = vbCapacity + minSize + GROW_FACTOR;
		RHI::BufferDesc desc;
		desc.size = new_size;
		desc.usage = RHI::BufferUsage::VertexBuffer | RHI::BufferUsage::CopyDst | RHI::BufferUsage::CopySrc;
		RHI::AutomaticAllocationInfo allocInfo;
		allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		RHI::Ptr<RHI::Buffer> newVB = RendererBase::device->CreateBuffer(desc, 0, 0, &allocInfo, 0, RHI::ResourceType::Automatic).value();
		RHI::BufferMemoryBarrier barr;
		barr.AccessFlagsBefore = RHI::ResourceAcessFlags::TRANSFER_WRITE;
		barr.AccessFlagsAfter = RHI::ResourceAcessFlags::TRANSFER_READ;
		barr.buffer = meshVertices;
		barr.nextQueue = barr.previousQueue = RHI::QueueFamily::Ignored;
		barr.size = vbCapacity;
		barr.offset = 0;
		RendererBase::stagingCommandList->PipelineBarrier(RHI::PipelineStage::TRANSFER_BIT, RHI::PipelineStage::TRANSFER_BIT, 1,&barr,0,0);
		//Queue it with staging stuff
		RendererBase::stagingCommandList->CopyBufferRegion(0, 0, vbCapacity, meshVertices, newVB);
		barr.AccessFlagsAfter = RHI::ResourceAcessFlags::TRANSFER_WRITE;
		barr.buffer = newVB;
		RendererBase::stagingCommandList->PipelineBarrier(RHI::PipelineStage::TRANSFER_BIT, RHI::PipelineStage::TRANSFER_BIT, 1,&barr,0,0);
		//wait until copy is finished?
		RendererBase::FlushStagingBuffer();
		//before destroying old buffer, wait for old frames to render
		RendererBase::mainFence->Wait(RendererBase::currentFenceVal);
		meshVertices = newVB;
		vbFreeSpace += minSize + GROW_FACTOR;
		vbFreeFastSpace += minSize + GROW_FACTOR;
		vbCapacity = new_size;
		vbFreeList.Grow(new_size);
	}
	void Renderer::GrowMeshIndexBuffer(uint32_t minSize)
	{
		const uint32_t GROW_FACTOR = 20; //probably use a better, more size dependent method to determing this
		uint32_t new_size = ibCapacity + minSize + GROW_FACTOR;
		RHI::BufferDesc desc;
		desc.size = new_size;
		desc.usage = RHI::BufferUsage::IndexBuffer | RHI::BufferUsage::CopyDst | RHI::BufferUsage::CopySrc;
		
		RHI::AutomaticAllocationInfo allocInfo;
		allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		RHI::Ptr<RHI::Buffer> newIB = RendererBase::device->CreateBuffer(desc, 0, 0, &allocInfo, 0, RHI::ResourceType::Automatic).value();
		//Queue it with staging stuff
		RHI::BufferMemoryBarrier barr;
		barr.AccessFlagsBefore = RHI::ResourceAcessFlags::TRANSFER_WRITE;
		barr.AccessFlagsAfter = RHI::ResourceAcessFlags::TRANSFER_READ;
		barr.buffer = meshIndices;
		barr.nextQueue = barr.previousQueue = RHI::QueueFamily::Ignored;
		barr.size = ibCapacity;
		barr.offset = 0;
		RendererBase::stagingCommandList->PipelineBarrier(RHI::PipelineStage::TRANSFER_BIT, RHI::PipelineStage::TRANSFER_BIT, 1,&barr,0,0);
		RendererBase::stagingCommandList->CopyBufferRegion(0, 0, ibCapacity, meshIndices, newIB);
		barr.AccessFlagsAfter = RHI::ResourceAcessFlags::TRANSFER_WRITE;
		barr.buffer = newIB;
		RendererBase::stagingCommandList->PipelineBarrier(RHI::PipelineStage::TRANSFER_BIT, RHI::PipelineStage::TRANSFER_BIT, 1,&barr,0,0);
		//wait until copy is finished?
		RendererBase::FlushStagingBuffer();
		//before destroying old buffer, wait for old frames to render
		RendererBase::mainFence->Wait(RendererBase::currentFenceVal);
		meshIndices = newIB;
		ibFreeSpace += minSize + GROW_FACTOR;
		ibFreeFastSpace += minSize + GROW_FACTOR;
		ibCapacity = new_size;
		ibFreeList.Grow(new_size);
	}
	void Pistachio::Renderer::GrowConstantBuffer(uint32_t minExtraSize)
	{
		const uint32_t GROW_FACTOR = 20; //probably use a better, more size dependent method to determing this
		uint32_t new_size = cbCapacity + minExtraSize + GROW_FACTOR;
		RHI::BufferDesc desc;
		desc.size = new_size;
		desc.usage = RHI::BufferUsage::ConstantBuffer;
		RendererBase::mainFence->Wait(RendererBase::currentFenceVal);
		for (uint32_t i = 0; i < 3; i++)
		{
			
			RHI::AutomaticAllocationInfo allocInfo;
			allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::Sequential;
			RHI::Ptr<RHI::Buffer> newCB = RendererBase::device->CreateBuffer(desc, 0, 0, &allocInfo, 0, RHI::ResourceType::Automatic).value();
			void* writePtr;
			void* readPtr;
			newCB->Map(&writePtr);
			resources[i].transformBuffer.ID->Map(&readPtr);
			memcpy(writePtr, readPtr, cbCapacity);
			resources[i].transformBuffer.ID->UnMap();
			newCB->UnMap();
			resources[i].transformBuffer.ID = newCB;
		}
	}
	void Pistachio::Renderer::FreeVertexBuffer(const RendererVBHandle handle)
	{
		vbFreeSpace += handle.size;
		vbFreeList.DeAllocate(vbHandleOffsets[handle.handle], handle.size);
		vbUnusedHandles.push_back(handle.handle);
	}
	void Renderer::FreeIndexBuffer(const RendererIBHandle handle)
	{
		ibFreeSpace += handle.size;
		ibFreeList.DeAllocate(ibHandleOffsets[handle.handle], handle.size);
		ibUnusedHandles.push_back(handle.handle);
	}
	RHI::Ptr<RHI::Buffer> Renderer::GetVertexBuffer()
	{
		return meshVertices;
	}
	RHI::Ptr<RHI::Buffer> Renderer::GetIndexBuffer()
	{
		return meshIndices;
	}
	void Renderer::DefragmentMeshVertexBuffer()
	{
		auto block = vbFreeList.GetBlockPtr();
		uint32_t nextFreeOffset = 0;
		while (block)
		{
			if (block->offset > nextFreeOffset)
			{
				RendererBase::stagingCommandList->CopyBufferRegion(block->offset, nextFreeOffset, block->size, meshVertices, meshVertices);
				nextFreeOffset += block->size;
			}
			block = block->next;
		}
		vbFreeList.Reset();
		//fill up the beginning of the free list with the memory size we just copied
		vbFreeList.Allocate(0, nextFreeOffset);
		vbFreeFastSpace = vbFreeSpace;//after defrag all free space is fast space
		/*
		 *Is that a safe assumptions;
		 *we dont flush for every copy, because we asssume copies are done in order, so memory won't get overritten
		 */
		RendererBase::FlushStagingBuffer();
		
	}
	void Renderer::DefragmentMeshIndexBuffer()
	{
		auto block = ibFreeList.GetBlockPtr();
		uint32_t nextFreeOffset = 0;
		while (block)
		{
			if (block->offset > nextFreeOffset)
			{
				RendererBase::stagingCommandList->CopyBufferRegion(block->offset, nextFreeOffset, block->size, meshIndices, meshIndices);
				nextFreeOffset += block->size;
			}
			block = block->next;
		}
		ibFreeList.Reset();
		//fill up the beginning of the free list with the memory size we just copied
		ibFreeList.Allocate(0, nextFreeOffset);
		ibFreeFastSpace = vbFreeSpace;//after defrag all free space is fast space
		/*
		 *Is that a safe assumptions;
		 *we dont flush for every copy, because we asssume copies are done in order, so memory won't get overritten
		 */
		RendererBase::FlushStagingBuffer();

	}
	void Renderer::DefragmentConstantBuffer()
	{
		RendererBase::mainFence->Wait(RendererBase::currentFenceVal);
		auto block = cbFreeList.GetBlockPtr();
		uint32_t nextFreeOffset = 0;
		void *ptr1, *ptr2, *ptr3;
		resources[0].transformBuffer.ID->Map(&ptr1);
		resources[1].transformBuffer.ID->Map(&ptr2);
		resources[2].transformBuffer.ID->Map(&ptr3);
		while (block)
		{
			//if block has gap from last block
			if (block->offset > nextFreeOffset)
			{
				//we use memmove to handle overlap possibility
				memmove((uint8_t*)ptr1 + nextFreeOffset, (uint8_t*)ptr1 + block->offset, block->size);
				memmove((uint8_t*)ptr2 + nextFreeOffset, (uint8_t*)ptr2 + block->offset, block->size);
				memmove((uint8_t*)ptr3 + nextFreeOffset, (uint8_t*)ptr3 + block->offset, block->size);
				nextFreeOffset += block->size;
			}
			block = block->next;
		}
		resources[0].transformBuffer.ID->UnMap();
		resources[1].transformBuffer.ID->UnMap();
		resources[2].transformBuffer.ID->UnMap();
	}
	const uint32_t Pistachio::Renderer::GetIBOffset(const RendererIBHandle handle)
	{
		return ibHandleOffsets[handle.handle];
	}
	const uint32_t Pistachio::Renderer::GetVBOffset(const RendererVBHandle handle)
	{
		return vbHandleOffsets[handle.handle];
	}
	void Pistachio::Renderer::PartialCBUpdate(RendererCBHandle handle, void* data, uint32_t offset, uint32_t size)
	{
		PT_CORE_ASSERT(offset+size <= handle.size);
		resources[RendererBase::currentFrameIndex].transformBuffer.Update(data, size, cbHandleOffsets[handle.handle] + offset);
	}
	void Pistachio::Renderer::FullCBUpdate(RendererCBHandle handle, void* data)
	{
		resources[RendererBase::currentFrameIndex].transformBuffer.Update(data, handle.size, cbHandleOffsets[handle.handle]);
	}
	RHI::Ptr<RHI::Buffer> Pistachio::Renderer::GetConstantBuffer()
	{
		return resources[RendererBase::currentFrameIndex].transformBuffer.ID;
	}
	const RHI::Ptr<RHI::DynamicDescriptor> Pistachio::Renderer::GetCBDesc()
	{
		return resources[RendererBase::currentFrameIndex].transformBufferDesc;
	}
	const RHI::Ptr<RHI::DynamicDescriptor> Renderer::GetCBDescPS()
	{
		return resources[RendererBase::currentFrameIndex].transformBufferDescPS;
	}
	const uint32_t Pistachio::Renderer::GetCBOffset(const RendererCBHandle handle)
	{
		return cbHandleOffsets[handle.handle];
	}
	void Pistachio::Renderer::Submit(RHI::Weak<RHI::GraphicsCommandList> list,const RendererVBHandle vb, const RendererIBHandle ib, uint32_t vertexStride)
	{
		list->DrawIndexed(ib.size / sizeof(uint32_t),
			1,
			ibHandleOffsets[ib.handle] / sizeof(uint32_t),
			vbHandleOffsets[vb.handle] / vertexStride, 0);
	}
	const Texture2D& Pistachio::Renderer::GetWhiteTexture()
	{
		return whiteTexture;
	}
	SamplerHandle Pistachio::Renderer::GetDefaultSampler()
	{
		return defaultSampler;
	}
}