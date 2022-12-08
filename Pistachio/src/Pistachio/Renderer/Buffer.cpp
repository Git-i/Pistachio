#include "ptpch.h"
#include "Buffer.h"
#include "DirectX11/DX11Buffer.h"

namespace Pistachio {
	VertexBuffer::VertexBuffer()
	{
	}
	VertexBuffer::~VertexBuffer()
	{
		#ifdef PISTACHIO_RENDER_API_DX11
			pVertexBuffer->Release();
		#endif // PISTACHIO_RENDER_API_DX11 
	}
	void VertexBuffer::Bind()
	{
		#ifdef PISTACHIO_RENDER_API_DX11
			UINT offset = 0;
			Renderer::g_pd3dDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
		#endif // PISTACHIO_RENDER_API_DX11
	}
	void VertexBuffer::UnBind()
	{
	}
	void VertexBuffer::Initialize(const void* vertices, unsigned int size, unsigned int Stride)
	{
		stride = Stride;
		#ifdef PISTACHIO_RENDER_API_DX11
			pVertexBuffer = DX11Buffer::CreateVertexBuffer(vertices, size, Stride);
		#endif
	}
	
	IndexBuffer::IndexBuffer()
	{
	}
	IndexBuffer::~IndexBuffer()
	{
		#ifdef PISTACHIO_RENDER_API_DX11
			pIndexBuffer->Release();
		#endif // PISTACHIO_RENDER_API_DX11

	}
	void IndexBuffer::Bind()
	{
		#ifdef PISTACHIO_RENDER_API_DX11
			Renderer::g_pd3dDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		#endif // PISTACHIO_RENDER_API_DX11
	}
	void IndexBuffer::UnBind()
	{
	}
	void IndexBuffer::Initialize(const void* indices, unsigned int size, unsigned int stride)
	{
		
		#ifdef PISTACHIO_RENDER_API_DX11
			pIndexBuffer = DX11Buffer::CreateIndexBuffer(indices, size, stride);
		#endif // PISTACHIO_RENDER_API_DX11

	}
}