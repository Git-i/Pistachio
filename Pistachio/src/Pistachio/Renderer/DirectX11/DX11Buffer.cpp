#include "ptpch.h"
#include "../Buffer.h"
#include "../RendererBase.h"

#define BUFFER(ID) ((ID3D11Buffer*)ID.Get())
#define BUFFER_PP(ID) ((ID3D11Buffer**)ID.GetAddressOf())
namespace Pistachio {
	VertexBuffer::VertexBuffer()
	{
	}
	void VertexBuffer::Bind() const
	{
		UINT offset = 0;
		RendererBase::Getd3dDeviceContext()->IASetVertexBuffers(0, 1, BUFFER_PP(ID), &stride, &offset);
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
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		bd.ByteWidth = size;
		RendererBase::Getd3dDevice()->CreateBuffer(&bd, nullptr, BUFFER_PP(ID));
	}
	void VertexBuffer::SetData(const void* data, unsigned int size)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
		RendererBase::Getd3dDeviceContext()->Map(BUFFER(ID), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, data, size);
		RendererBase::Getd3dDeviceContext()->Unmap(BUFFER(ID), 0);
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
		stride = Stride;
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.ByteWidth = size;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices;
		RendererBase::Getd3dDevice()->CreateBuffer(&bd, &sd, (ID3D11Buffer**)(ID.ReleaseAndGetAddressOf()));
	}

	IndexBuffer::IndexBuffer()
	{
	}
	void IndexBuffer::Bind() const
	{
		RendererBase::Getd3dDeviceContext()->IASetIndexBuffer(BUFFER(ID), DXGI_FORMAT_R32_UINT, 0);
	}
	void IndexBuffer::UnBind()
	{
	}
	void IndexBuffer::CreateStack(const void* indices, unsigned int size, unsigned int stride)
	{
		PT_PROFILE_FUNCTION()
		count = size / stride;
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.ByteWidth = size;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = indices;
		RendererBase::Getd3dDevice()->CreateBuffer(&bd, &sd, (ID3D11Buffer**)(ID.ReleaseAndGetAddressOf()));
	}
	IndexBuffer* IndexBuffer::Create(const void* indices, unsigned int size, unsigned int stride)
	{
		IndexBuffer* result = new IndexBuffer;
		result->CreateStack(indices, size, stride);
		return result;
	}
}