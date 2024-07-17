#pragma once
#include "CommandList.h"
#include "Core/Buffer.h"
#include "Pistachio/Core/Error.h"
namespace Pistachio {
	class PISTACHIO_API VertexBuffer
	{
	public:
		void Bind(RHI::GraphicsCommandList* list, uint32_t slot = 0) const;

		VertexBuffer();
		void SetData(const void* data, unsigned int size);
		static Result<VertexBuffer*> Create(const void* vertices, unsigned int size, unsigned int stride);
		Error CreateStack(const void* vertices, unsigned int size, unsigned int stride);
		
		uint32_t GetStride() const {return stride;}
		RHI::Ptr<RHI::Buffer> GetID() const {return ID;}
	private:
		friend class Renderer;
		uint32_t stride = 0;
		RHI::Ptr<RHI::Buffer> ID;
	};
	class PISTACHIO_API IndexBuffer
	{
	public:
		void Bind(RHI::GraphicsCommandList* list) const;

		IndexBuffer();
		static Result<IndexBuffer*> Create(const void* indices, unsigned int size, unsigned int stride);
		Error CreateStack(const void* indices, unsigned int size, unsigned int stride);
		
		uint32_t GetCount() const{ return count; }
		RHI::Ptr<RHI::Buffer> GetID() const {return ID;}
	private:
		friend class Renderer;
		uint32_t count;
		RHI::Ptr<RHI::Buffer> ID;
	};
	enum class SBCreateFlags
	{
		None = 0,
		AllowCPUAccess = 1,
	};
	ENUM_FLAGS(SBCreateFlags);
	struct PISTACHIO_API StructuredBuffer
	{
		void Bind(std::uint32_t slot) const;
		void Update(const void* data, std::uint32_t size, std::uint32_t offset);
		static Result<StructuredBuffer*> Create(const void* data, std::uint32_t size, SBCreateFlags flags = SBCreateFlags::None);
		
		Error CreateStack(const void* data, std::uint32_t size, SBCreateFlags flags = SBCreateFlags::None);
		RHI::Ptr<RHI::Buffer> GetID() const { return ID; }
	private:
		friend class Renderer;
		RHI::Ptr<RHI::Buffer> ID;
	};
	class PISTACHIO_API ConstantBuffer {
	public:
		void Bind(std::uint32_t slot) const;
		void Update(void* data, std::uint32_t size, std::uint32_t offset);
		Error CreateStack(void* data, std::uint32_t size);
		static Result<ConstantBuffer*> Create(void* data, std::uint32_t size);
		
		RHI::Ptr<RHI::Buffer> GetID() const { return ID; }
	private:
		friend class Renderer;
		RHI::Ptr<RHI::Buffer> ID;
	};
}
