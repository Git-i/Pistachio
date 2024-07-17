#include "CommandList.h"
#include "Pistachio/Core/Error.h"
#include "ptpch.h"
#include "Buffer.h"
#include "RendererBase.h"

namespace Pistachio {
	VertexBuffer::VertexBuffer()
	{
	}
	void VertexBuffer::Bind(RHI::GraphicsCommandList* list, uint32_t slot) const
	{
		PT_PROFILE_FUNCTION();
		uint32_t offset = 0;
		list->BindVertexBuffers(slot, 1, &ID->ID);//id->id is a little confusing
	}
	Result<VertexBuffer*> VertexBuffer::Create(const void* verts, unsigned int size, unsigned int stride)
	{
		PT_PROFILE_FUNCTION();
		VertexBuffer* result = new VertexBuffer();
		auto res = result->CreateStack(verts, size, stride);
		if(res.GetErrorType() != ErrorType::Success) 
		{
			delete result;
			return ezr::err(std::move(res));
		}
		return ezr::ok(std::move(result));
	}
	Error VertexBuffer::CreateStack(const void* verts, unsigned int size, unsigned int Stride)
	{
		PT_PROFILE_FUNCTION();
		stride = Stride;
		RHI::BufferDesc vbDesc;
		vbDesc.size = size;
		vbDesc.usage = RHI::BufferUsage::VertexBuffer | RHI::BufferUsage::CopyDst;
		RHI::AutomaticAllocationInfo info;
		info.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		auto res = RendererBase::Getd3dDevice()->CreateBuffer(vbDesc, nullptr, nullptr,&info, 0, RHI::ResourceType::Automatic);
		if(res.is_err())
		{
			return Error::FromRHIError(res.err());
		}
		ID = std::move(res).value();
		if(verts) RendererBase::PushBufferUpdate(ID, 0, verts, size);
		return Error::None();
	}
	void VertexBuffer::SetData(const void* data, unsigned int size)
	{
		PT_PROFILE_FUNCTION();
		RendererBase::PushBufferUpdate(ID, 0, data, size);
	}

	IndexBuffer::IndexBuffer()
	{
	}
	void IndexBuffer::Bind(RHI::GraphicsCommandList* list) const
	{
		PT_PROFILE_FUNCTION();
		list->BindIndexBuffer(ID, 0);
	}
	Error IndexBuffer::CreateStack(const void* indices, unsigned int size, unsigned int stride)
	{
		PT_PROFILE_FUNCTION()
		count = size / stride;
		RHI::BufferDesc desc;
		desc.size = size;
		desc.usage = RHI::BufferUsage::IndexBuffer | RHI::BufferUsage::CopyDst;
		RHI::AutomaticAllocationInfo info;
		info.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		auto res = RendererBase::Getd3dDevice()->CreateBuffer(desc, nullptr, nullptr, &info,0, RHI::ResourceType::Automatic);
		if(res.is_err()) return Error::FromRHIError(res.err());
		if (indices)
		{
			RendererBase::PushBufferUpdate(ID, 0, indices, size);
		}
		ID = std::move(res).value();
		return Error::None();
	}
	Result<IndexBuffer*> IndexBuffer::Create(const void* indices, std::uint32_t size, std::uint32_t stride)
	{
		PT_PROFILE_FUNCTION();
		IndexBuffer* result = new IndexBuffer;
		auto res = result->CreateStack(indices, size,stride);
		if(res.GetErrorType() != ErrorType::Success)
		{
			delete result;
			return ezr::err(std::move(res));
		}
		return ezr::ok(std::move(result));
	}
	Error StructuredBuffer::CreateStack(const void* data, std::uint32_t size, SBCreateFlags flags)
	{
		PT_PROFILE_FUNCTION();
		RHI::BufferDesc bufferDesc;
		bufferDesc.size = size;
		bufferDesc.usage = RHI::BufferUsage::StructuredBuffer | RHI::BufferUsage::CopyDst;
		RHI::AutomaticAllocationInfo info;
		info.access_mode = ((flags & SBCreateFlags::AllowCPUAccess)==SBCreateFlags::None) ? 
			RHI::AutomaticAllocationCPUAccessMode::None :
			RHI::AutomaticAllocationCPUAccessMode::Sequential;
		auto res = RendererBase::Getd3dDevice()->CreateBuffer(bufferDesc, nullptr, nullptr, &info, 0, RHI::ResourceType::Automatic);
		if(res.is_err()) return Error::FromRHIError(res.err());
		ID = std::move(res).value();
		if (data)
		{
			void* writePointer;
			ID->Map(&writePointer);
			memcpy(writePointer, data, size);
			ID->UnMap();
		}
		return Error::None();
		
	}
	void StructuredBuffer::Bind(std::uint32_t slot) const
	{
		//todo
	}
	void StructuredBuffer::Update(const void* data, std::uint32_t size, std::uint32_t offset)
	{
		void* writePointer;
		ID->Map(&writePointer);
		memcpy((((char*)writePointer) + offset), data, size);
		ID->UnMap();
	}
	Result<StructuredBuffer*> StructuredBuffer::Create(const void* data, std::uint32_t size, SBCreateFlags flags)
	{
		PT_PROFILE_FUNCTION();
		StructuredBuffer* result = new StructuredBuffer;
		auto res = result->CreateStack(data, size);
		if(res.GetErrorType() != ErrorType::Success)
		{
			delete result;
			return ezr::err(std::move(res));
		} 
		return ezr::ok(std::move(result));
	}
	void ConstantBuffer::Update(void* data, std::uint32_t size, std::uint32_t offset)
	{
		PT_PROFILE_FUNCTION();
		void* writePointer;
		ID->Map(&writePointer);
		memcpy((((char*)writePointer) + offset), data, size);
		ID->UnMap();
	}
	Error ConstantBuffer::CreateStack(void* data, std::uint32_t size)
	{
		PT_PROFILE_FUNCTION();
		RHI::BufferDesc bufferDesc;
		bufferDesc.size = size;
		bufferDesc.usage = RHI::BufferUsage::ConstantBuffer;
		RHI::AutomaticAllocationInfo info;
		info.access_mode = RHI::AutomaticAllocationCPUAccessMode::Sequential;
		auto res = RendererBase::Getd3dDevice()->CreateBuffer(bufferDesc, nullptr, nullptr, &info, 0, RHI::ResourceType::Automatic);
		if(res.is_err()) return Error::FromRHIError(res.err());
		ID = std::move(res).value();
		if (data)
		{
			void* writePointer;
			ID->Map(&writePointer);
			memcpy(writePointer, data, size);
			ID->UnMap();
		}
		return Error::None();
	}
	Result<ConstantBuffer*> ConstantBuffer::Create(void* data, std::uint32_t size)
	{
		PT_PROFILE_FUNCTION();
		ConstantBuffer* retVal = new ConstantBuffer();
		auto res  = retVal->CreateStack(data, size);
		if(res.GetErrorType() != ErrorType::Success)
		{ 
			delete retVal;
			return ezr::err(std::move(res));
		}	
		return ezr::ok(std::move(retVal));
	}
}