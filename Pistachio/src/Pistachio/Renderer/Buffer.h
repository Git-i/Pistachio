#pragma once
#include "RendererID_t.h"
namespace Pistachio {
	class VertexBuffer
	{
	public:
		VertexBuffer();
		void Bind() const;
		void UnBind();
		static VertexBuffer* Create(unsigned int size, unsigned int stride);
		void CreateStack(unsigned int size, unsigned int stride);
		void SetData(const void* data, unsigned int size);
		static VertexBuffer* Create(const void* vertices, unsigned int size, unsigned int stride);
		void CreateStack(const void* vertices, unsigned int size, unsigned int stride);
		
	private:
		unsigned int stride = 0;
		PlatformRendererID_t ID;
	};
	class IndexBuffer
	{
	public:
		IndexBuffer();
		void Bind() const;
		void UnBind();
		static IndexBuffer* Create(const void* indices, unsigned int size, unsigned int stride);
		void CreateStack(const void* indices, unsigned int size, unsigned int stride);
		inline unsigned int GetCount() const{ return count; }
	private:
		unsigned int count;
		PlatformRendererID_t ID;
	};
	struct Buffer
	{
	public:
		const VertexBuffer* vb;
		const IndexBuffer* ib;
		Buffer(const VertexBuffer* Vb, const IndexBuffer* Ib) : vb(Vb), ib(Ib) {}
		~Buffer(){}
		inline const void Bind() const
		{
			vb->Bind();
			ib->Bind();
		}
	};
}
