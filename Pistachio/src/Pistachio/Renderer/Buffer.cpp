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
			RendererBase::Getd3dDeviceContext()->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
		#endif // PISTACHIO_RENDER_API_DX11
	}
	void VertexBuffer::UnBind()
	{
	}
	VertexBuffer* VertexBuffer::Create(const void* vertices, unsigned int size, unsigned int Stride)
	{
		VertexBuffer* result = new VertexBuffer;
		result->stride = Stride;
		#ifdef PISTACHIO_RENDER_API_DX11
			result->pVertexBuffer = DX11Buffer::CreateVertexBuffer(vertices, size, Stride);
		#endif
		return result;
	}
	VertexBuffer VertexBuffer::CreateStack(const void* vertices, unsigned int size, unsigned int Stride)
	{
		VertexBuffer result;
		result.stride = Stride;
		#ifdef PISTACHIO_RENDER_API_DX11
			result.pVertexBuffer = DX11Buffer::CreateVertexBuffer(vertices, size, Stride);
		#endif
		return result;
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
			RendererBase::Getd3dDeviceContext()->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
		#endif // PISTACHIO_RENDER_API_DX11
	}
	void IndexBuffer::UnBind()
	{
	}
	IndexBuffer IndexBuffer::CreateStack(const void* indices, unsigned int size, unsigned int stride)
	{
		IndexBuffer result;
		result.count = size / stride;
		#ifdef PISTACHIO_RENDER_API_DX11
			result.pIndexBuffer = DX11Buffer::CreateIndexBuffer(indices, size, stride);
		#endif // PISTACHIO_RENDER_API_DX11
			return result;
	}	
	IndexBuffer* IndexBuffer::Create(const void* indices, unsigned int size, unsigned int stride)
	{
		IndexBuffer* result = new IndexBuffer;
		result->count = size / stride;
		#ifdef PISTACHIO_RENDER_API_DX11
			result->pIndexBuffer = DX11Buffer::CreateIndexBuffer(indices, size, stride);
		#endif // PISTACHIO_RENDER_API_DX11
			return result;
	}
}