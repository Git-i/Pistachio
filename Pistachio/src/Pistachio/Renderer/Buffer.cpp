#include "ptpch.h"
#include "Buffer.h"
#include "DirectX11/DX11Buffer.h"
#include "../Core/Error.h"
namespace Pistachio {
	VertexBuffer::VertexBuffer()
	{
	}
	void VertexBuffer::ShutDown()
	{
		#ifdef PISTACHIO_RENDER_API_DX11
		if (pVertexBuffer) {
			while (pVertexBuffer->Release()) {};
			pVertexBuffer = NULL;
		}
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
	VertexBuffer* VertexBuffer::Create(unsigned int size, unsigned int stride)
	{
		VertexBuffer* result = new VertexBuffer();
		result->CreateStack(size, stride);
		return result;
	}
	void VertexBuffer::CreateStack(unsigned int size, unsigned int Stride)
	{
		stride = Stride;
		DX11Buffer::CreateVertexBuffer(size, &pVertexBuffer);
	}
	void VertexBuffer::SetData(const void* data, unsigned int size)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		RendererBase::Getd3dDeviceContext()->Map(pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, data, size);
		RendererBase::Getd3dDeviceContext()->Unmap(pVertexBuffer, 0);
	}
	VertexBuffer* VertexBuffer::Create(const void* vertices, unsigned int size, unsigned int Stride)
	{
		PT_PROFILE_FUNCTION()
		VertexBuffer* result = new VertexBuffer;
		result->stride = Stride;
		#ifdef PISTACHIO_RENDER_API_DX11
			 Error::LogErrorToConsole(DX11Buffer::CreateVertexBuffer(vertices, size, Stride, &result->pVertexBuffer));
		#endif
		return result;
	}
	void VertexBuffer::CreateStack(const void* vertices, unsigned int size, unsigned int Stride)
	{
		PT_PROFILE_FUNCTION()
		stride = Stride;
		#ifdef PISTACHIO_RENDER_API_DX11
			Error::LogErrorToConsole(DX11Buffer::CreateVertexBuffer(vertices, size, Stride, &pVertexBuffer));
		#endif
	}
	
	IndexBuffer::IndexBuffer()
	{
	}
	void IndexBuffer::ShutDown()
	{
		PT_PROFILE_FUNCTION()
		#ifdef PISTACHIO_RENDER_API_DX11
		if (pIndexBuffer) {
			UnBind();
			while (pIndexBuffer->Release()) {};
			pIndexBuffer = NULL;
		}
		#endif // PISTACHIO_RENDER_API_DX11

	}
	void IndexBuffer::Bind()
	{
		#ifdef PISTACHIO_RENDER_API_DX11
			RendererBase::Getd3dDeviceContext()->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		#endif // PISTACHIO_RENDER_API_DX11
	}
	void IndexBuffer::UnBind()
	{
	}
	void IndexBuffer::CreateStack(const void* indices, unsigned int size, unsigned int stride)
	{
		PT_PROFILE_FUNCTION()
		count = size / stride;
		#ifdef PISTACHIO_RENDER_API_DX11
			Error::LogErrorToConsole(DX11Buffer::CreateIndexBuffer(indices, size, stride, &pIndexBuffer));
		#endif // PISTACHIO_RENDER_API_DX11
	}	
	IndexBuffer* IndexBuffer::Create(const void* indices, unsigned int size, unsigned int stride)
	{
		IndexBuffer* result = new IndexBuffer;
		result->count = size / stride;
		#ifdef PISTACHIO_RENDER_API_DX11
			Error::LogErrorToConsole(DX11Buffer::CreateIndexBuffer(indices, size, stride, &result->pIndexBuffer));
		#endif // PISTACHIO_RENDER_API_DX11
			return result;
	}
}