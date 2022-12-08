#pragma once
#include "RendererBase.h"

namespace Pistachio {

#ifdef PISTACHIO_RENDER_API_DX11
	
#endif // PISTACHIO_RENDER_API_DX11

	
	class VertexBuffer
	{
	public:
		VertexBuffer();
		~VertexBuffer();
		void Bind();
		void UnBind();
		void Initialize(const void* vertices, unsigned int size, unsigned int stride);
		
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
		~IndexBuffer();
		void Bind();
		void UnBind();
		void Initialize(const void* indices, unsigned int size, unsigned int stride);
	private:
		#ifdef PISTACHIO_RENDER_API_DX11
			ID3D11Buffer* pIndexBuffer = NULL;
		#endif // PISTACHIO_RENDER_API_DX11
	};
}
