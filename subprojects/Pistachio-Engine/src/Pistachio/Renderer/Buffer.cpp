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
	creation_result<VertexBuffer*> VertexBuffer::Create(const void* verts, unsigned int size, unsigned int stride)
	{
		PT_PROFILE_FUNCTION();
		VertexBuffer* result = new VertexBuffer();
		auto res = result->CreateStack(verts, size, stride);
		if(res.is_err()) delete result;
		return res.transform([&]()->VertexBuffer*{return result;});
	}
	init_result VertexBuffer::CreateStack(const void* verts, unsigned int size, unsigned int Stride)
	{
		PT_PROFILE_FUNCTION();
		stride = Stride;
		RHI::BufferDesc vbDesc;
		vbDesc.size = size;
		vbDesc.usage = RHI::BufferUsage::VertexBuffer | RHI::BufferUsage::CopyDst;
		RHI::AutomaticAllocationInfo info;
		info.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		auto res = RendererBase::Getd3dDevice()->CreateBuffer(&vbDesc, nullptr, nullptr,&info, 0, RHI::ResourceType::Automatic);
		if(res.is_err())
		{
			return init_result::err(Error::FromRHIError(res.err()));
		}
		ID = std::move(res).value();
		if(verts) RendererBase::PushBufferUpdate(ID.Get(), 0, verts, size);
		return init_result::ok();
	}
	void VertexBuffer::SetData(const void* data, unsigned int size)
	{
		PT_PROFILE_FUNCTION();
		RendererBase::PushBufferUpdate(ID.Get(), 0, data, size);
	}

	IndexBuffer::IndexBuffer()
	{
	}
	void IndexBuffer::Bind(RHI::GraphicsCommandList* list) const
	{
		PT_PROFILE_FUNCTION();
		list->BindIndexBuffer(ID.Get(), 0);
	}
	init_result IndexBuffer::CreateStack(const void* indices, unsigned int size, unsigned int stride)
	{
		PT_PROFILE_FUNCTION()
		count = size / stride;
		RHI::BufferDesc desc;
		desc.size = size;
		desc.usage = RHI::BufferUsage::IndexBuffer | RHI::BufferUsage::CopyDst;
		RHI::AutomaticAllocationInfo info;
		info.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		auto res = RendererBase::Getd3dDevice()->CreateBuffer(&desc, nullptr, nullptr, &info,0, RHI::ResourceType::Automatic);
		if(res.is_err()) return init_result::err(Error::FromRHIError(res.err()));
		if (indices)
		{
			RendererBase::PushBufferUpdate(ID.Get(), 0, indices, size);
		}
		ID = std::move(res).value();
		return init_result::ok();
	}
	creation_result<IndexBuffer*> IndexBuffer::Create(const void* indices, std::uint32_t size, std::uint32_t stride)
	{
		PT_PROFILE_FUNCTION();
		IndexBuffer* result = new IndexBuffer;
		auto res = result->CreateStack(indices, size,stride);
		if(res.is_err()) delete result;
		return res.transform([&]{return result;});
	}
	init_result StructuredBuffer::CreateStack(const void* data, std::uint32_t size, SBCreateFlags flags)
	{
		PT_PROFILE_FUNCTION();
		RHI::BufferDesc bufferDesc;
		bufferDesc.size = size;
		bufferDesc.usage = RHI::BufferUsage::StructuredBuffer | RHI::BufferUsage::CopyDst;
		RHI::AutomaticAllocationInfo info;
		info.access_mode = ((flags & SBCreateFlags::AllowCPUAccess)==SBCreateFlags::None) ? 
			RHI::AutomaticAllocationCPUAccessMode::None :
			RHI::AutomaticAllocationCPUAccessMode::Sequential;
		auto res = RendererBase::Getd3dDevice()->CreateBuffer(&bufferDesc, nullptr, nullptr, &info, 0, RHI::ResourceType::Automatic);
		if(res.is_err()) return init_result::err(Error::FromRHIError(res.err()));
		ID = std::move(res).value();
		if (data)
		{
			void* writePointer;
			ID->Map(&writePointer);
			memcpy(writePointer, data, size);
			ID->UnMap();
		}
		return init_result::ok();
		
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
	creation_result<StructuredBuffer*> StructuredBuffer::Create(const void* data, std::uint32_t size, SBCreateFlags flags)
	{
		PT_PROFILE_FUNCTION();
		StructuredBuffer* result = new StructuredBuffer;
		auto res = result->CreateStack(data, size);
		if(res.is_err()) delete result;
		return res.transform([&]{return result;});
	}
	void ConstantBuffer::Update(void* data, std::uint32_t size, std::uint32_t offset)
	{
		PT_PROFILE_FUNCTION();
		void* writePointer;
		ID->Map(&writePointer);
		memcpy((((char*)writePointer) + offset), data, size);
		ID->UnMap();
	}
	init_result ConstantBuffer::CreateStack(void* data, std::uint32_t size)
	{
		PT_PROFILE_FUNCTION();
		RHI::BufferDesc bufferDesc;
		bufferDesc.size = size;
		bufferDesc.usage = RHI::BufferUsage::ConstantBuffer;
		RHI::AutomaticAllocationInfo info;
		info.access_mode = RHI::AutomaticAllocationCPUAccessMode::Sequential;
		auto res = RendererBase::Getd3dDevice()->CreateBuffer(&bufferDesc, nullptr, nullptr, &info, 0, RHI::ResourceType::Automatic);
		if(res.is_err()) return init_result::err(Error::FromRHIError(res.err()));
		ID = std::move(res).value();
		if (data)
		{
			void* writePointer;
			ID->Map(&writePointer);
			memcpy(writePointer, data, size);
			ID->UnMap();
		}
		return init_result::ok();
	}
	creation_result<ConstantBuffer*> ConstantBuffer::Create(void* data, std::uint32_t size)
	{
		PT_PROFILE_FUNCTION();
		ConstantBuffer* retVal = new ConstantBuffer();
		auto res  = retVal->CreateStack(data, size);
		if(res.is_err()) delete retVal;
		return res.transform([&]{return retVal;});
	}
}