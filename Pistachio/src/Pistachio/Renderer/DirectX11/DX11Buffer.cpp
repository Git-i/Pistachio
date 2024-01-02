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
		PT_PROFILE_FUNCTION();
		UINT offset = 0;
		RendererBase::Getd3dDeviceContext()->IASetVertexBuffers(0, 1, BUFFER_PP(ID), &stride, &offset);
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
		PT_PROFILE_FUNCTION();
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
		PT_PROFILE_FUNCTION();
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
		PT_PROFILE_FUNCTION();
		IndexBuffer* result = new IndexBuffer;
		result->CreateStack(indices, size, stride);
		return result;
	}
	void StructuredBuffer::CreateStack(const void* data, std::uint32_t size, std::uint32_t stride)
	{
		PT_PROFILE_FUNCTION();
		ID3D11Buffer* buffer;
		D3D11_BUFFER_DESC bd;
		bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = size;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bd.StructureByteStride = stride;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = data;
		D3D11_SUBRESOURCE_DATA* sd_ptr = data ? &sd : nullptr;
		RendererBase::Getd3dDevice()->CreateBuffer(&bd, sd_ptr, &buffer);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.ElementOffset = 0;
		srvDesc.Buffer.ElementWidth = stride;
		srvDesc.Buffer.NumElements = size / stride;
		RendererBase::Getd3dDevice()->CreateShaderResourceView(buffer, &srvDesc, (ID3D11ShaderResourceView**)(ID.ReleaseAndGetAddressOf()));
		buffer->Release();
	}
	void StructuredBuffer::Bind(std::uint32_t slot) const
	{
		RendererBase::Getd3dDeviceContext()->PSSetShaderResources(slot, 1, (ID3D11ShaderResourceView**)ID.GetAddressOf());
	}
	void StructuredBuffer::Update(const void* data, std::uint32_t size)
	{
		ID3D11ShaderResourceView* srv = (ID3D11ShaderResourceView*)ID.Get();
		ID3D11Resource* res;
		srv->GetResource(&res);
		D3D11_MAPPED_SUBRESOURCE sr;
		RendererBase::Getd3dDeviceContext()->Map(res, 0, D3D11_MAP_WRITE_DISCARD, 0, &sr);
		memcpy(sr.pData, data, size);
		RendererBase::Getd3dDeviceContext()->Unmap(res, 0);
		res->Release();
	}
	StructuredBuffer* StructuredBuffer::Create(const void* data, std::uint32_t size, std::uint32_t stride)
	{
		PT_PROFILE_FUNCTION();
		StructuredBuffer* result = new StructuredBuffer;
		result->CreateStack(data, size, stride);
		return result;
	}
}