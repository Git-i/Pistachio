#pragma once

namespace Pistachio {
	class VertexBuffer
	{
	public:
		VertexBuffer();
		void ShutDown();
		void Bind();
		void UnBind();
		static VertexBuffer* Create(const void* vertices, unsigned int size, unsigned int stride);
		void CreateStack(const void* vertices, unsigned int size, unsigned int stride);
		
	private:
		unsigned int stride = 0;
		#ifdef PISTACHIO_RENDER_API_DX11
			ID3D11Buffer* pVertexBuffer = NULL;
		#endif // PISTACHIO_RENDER_API_DX11
	};
	class IndexBuffer
	{
	public:
		IndexBuffer();
		void ShutDown();
		void Bind();
		void UnBind();
		static IndexBuffer* Create(const void* indices, unsigned int size, unsigned int stride);
		void CreateStack(const void* indices, unsigned int size, unsigned int stride);
		inline unsigned int GetCount() { return count; }
	private:
		unsigned int count;
		#ifdef PISTACHIO_RENDER_API_DX11
			ID3D11Buffer* pIndexBuffer = NULL;
		#endif // PISTACHIO_RENDER_API_DX11
	};
	struct Buffer
	{
	public:
		VertexBuffer* vb;
		IndexBuffer* ib;
		Buffer(VertexBuffer* Vb, IndexBuffer* Ib) : vb(Vb), ib(Ib) {}
		inline const void Bind()
		{
			vb->Bind();
			ib->Bind();
		}
	};
}
