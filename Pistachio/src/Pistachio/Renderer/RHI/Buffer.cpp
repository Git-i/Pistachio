#include "ptpch.h"
#include "../Buffer.h"
#include "../RendererBase.h"

namespace Pistachio {
	VertexBuffer::VertexBuffer()
	{
	}
	void VertexBuffer::Bind() const
	{
		PT_PROFILE_FUNCTION();
		UINT offset = 0;
		RendererBase::GetMainCommandList()->BindVertexBuffers(0, 1, &ID->ID);//id->id is a little confusing
	}
	void VertexBuffer::UnBind()
	{
	}
	VertexBuffer* VertexBuffer::Create(unsigned int size, unsigned int stride)
	{
		PT_PROFILE_FUNCTION();
		VertexBuffer* result = new VertexBuffer();
		result->CreateStack(size, stride);
		return result;
	}
	void VertexBuffer::CreateStack(unsigned int size, unsigned int Stride)
	{
		PT_PROFILE_FUNCTION();
		stride = Stride;
		RHI::BufferDesc vbDesc;
		vbDesc.size = size;
		vbDesc.usage = RHI::BufferUsage::VertexBuffer | RHI::BufferUsage::CopyDst;
		RHI::AutomaticAllocationInfo info;
		info.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		RendererBase::Getd3dDevice()->CreateBuffer(&vbDesc, &ID, nullptr, nullptr,&info, 0, RHI::ResourceType::Automatic);
	}
	void VertexBuffer::SetData(const void* data, unsigned int size)
	{
		PT_PROFILE_FUNCTION();
		RendererBase::PushBufferUpdate(ID.Get(), 0, data, size);
	}
	VertexBuffer* VertexBuffer::Create(const void* vertices, unsigned int size, unsigned int Stride)
	{
		PT_PROFILE_FUNCTION()
		VertexBuffer* result = new VertexBuffer;
		result->CreateStack(vertices, size, Stride);
		return result;
	}
	void VertexBuffer::CreateStack(const void* vertices, unsigned int size, unsigned int Stride)
	{
		PT_PROFILE_FUNCTION()
		CreateStack(size, Stride);
		RendererBase::PushBufferUpdate(ID.Get(), 0, vertices, size);
	}

	IndexBuffer::IndexBuffer()
	{
	}
	void IndexBuffer::Bind() const
	{
		PT_PROFILE_FUNCTION();
		RendererBase::GetMainCommandList()->BindIndexBuffer(ID.Get(), 0);
	}
	void IndexBuffer::UnBind()
	{
	}
	void IndexBuffer::CreateStack(const void* indices, unsigned int size, unsigned int stride)
	{
		PT_PROFILE_FUNCTION()
		count = size / stride;
		RHI::BufferDesc desc;
		desc.size = size;
		desc.usage = RHI::BufferUsage::IndexBuffer | RHI::BufferUsage::CopyDst;
		RHI::AutomaticAllocationInfo info;
		info.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		RendererBase::Getd3dDevice()->CreateBuffer(&desc, &ID, nullptr, nullptr, &info,0, RHI::ResourceType::Automatic);
		if (indices)
		{
			RendererBase::PushBufferUpdate(ID.Get(), 0, indices, size);
		}
	}
	IndexBuffer* IndexBuffer::Create(const void* indices, std::uint32_t size, std::uint32_t stride)
	{
		PT_PROFILE_FUNCTION();
		IndexBuffer* result = new IndexBuffer;
		result->CreateStack(indices, size,stride);
		return result;
	}
	void StructuredBuffer::CreateStack(const void* data, std::uint32_t size, SBCreateFlags flags)
	{
		PT_PROFILE_FUNCTION();
		RHI::BufferDesc bufferDesc;
		bufferDesc.size = size;
		bufferDesc.usage = RHI::BufferUsage::StructuredBuffer | RHI::BufferUsage::CopyDst;
		RHI::AutomaticAllocationInfo info;
		info.access_mode = ((flags & SBCreateFlags::AllowCPUAccess)==SBCreateFlags::None) ? 
			RHI::AutomaticAllocationCPUAccessMode::None :
			RHI::AutomaticAllocationCPUAccessMode::Sequential;
		RendererBase::Getd3dDevice()->CreateBuffer(&bufferDesc, &ID, nullptr, nullptr, &info, 0, RHI::ResourceType::Automatic);
		if (data)
		{
			void* writePointer;
			ID->Map(&writePointer);
			memcpy(writePointer, data, size);
			ID->UnMap();
		}
		
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
	StructuredBuffer* StructuredBuffer::Create(const void* data, std::uint32_t size, SBCreateFlags flags)
	{
		PT_PROFILE_FUNCTION();
		StructuredBuffer* result = new StructuredBuffer;
		result->CreateStack(data, size);
		return result;
	}
	void ConstantBuffer::Update(void* data, std::uint32_t size, std::uint32_t offset)
	{
		PT_PROFILE_FUNCTION();
		void* writePointer;
		ID->Map(&writePointer);
		memcpy((((char*)writePointer) + offset), data, size);
		ID->UnMap();
	}
	void ConstantBuffer::CreateStack(void* data, std::uint32_t size)
	{
		PT_PROFILE_FUNCTION();
		RHI::BufferDesc bufferDesc;
		bufferDesc.size = size;
		bufferDesc.usage = RHI::BufferUsage::ConstantBuffer;
		RHI::AutomaticAllocationInfo info;
		info.access_mode = RHI::AutomaticAllocationCPUAccessMode::Sequential;
		RendererBase::Getd3dDevice()->CreateBuffer(&bufferDesc, &ID, nullptr, nullptr, &info, 0, RHI::ResourceType::Automatic);
		if (data)
		{
			void* writePointer;
			ID->Map(&writePointer);
			memcpy(writePointer, data, size);
			ID->UnMap();
		}

	}
	ConstantBuffer* ConstantBuffer::Create(void* data, std::uint32_t size)
	{
		PT_PROFILE_FUNCTION();
		ConstantBuffer* retVal = new ConstantBuffer();
		retVal->CreateStack(data, size);
		return retVal;
	}
}