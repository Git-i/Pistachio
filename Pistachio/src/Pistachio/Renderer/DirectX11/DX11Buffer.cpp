#include "ptpch.h"
#include "DX11Buffer.h"

namespace Pistachio {

	ID3D11Buffer* DX11Buffer::CreateVertexBuffer(const void* vertices, unsigned int size, unsigned int stride)
	{
		ID3D11Buffer* pVertexBuffer;
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.ByteWidth = size;
		bd.StructureByteStride = stride;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices;
		RendererBase::Getd3dDevice()->CreateBuffer(&bd, &sd, &pVertexBuffer);
		return pVertexBuffer;
	}
	ID3D11Buffer* DX11Buffer::CreateIndexBuffer(const void* indices, unsigned int size, unsigned int stride)
	{
		ID3D11Buffer* pIndexBuffer;
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.ByteWidth = size;
		bd.StructureByteStride = stride;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = indices;
		RendererBase::Getd3dDevice()->CreateBuffer(&bd, &sd, &pIndexBuffer);
		return pIndexBuffer;
	}
	
}