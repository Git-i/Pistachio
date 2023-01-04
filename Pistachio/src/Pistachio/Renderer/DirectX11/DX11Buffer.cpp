#include "ptpch.h"
#include "DX11Buffer.h"
#include "../../Core/Error.h"
namespace Pistachio {

	Error DX11Buffer::CreateVertexBuffer(const void* vertices, unsigned int size, unsigned int stride, ID3D11Buffer** pVertexBuffer)
	{
		if (!vertices)
			return Error(ErrorType::NullError, std::string(__FUNCTION__));
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.ByteWidth = size;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices;
		RendererBase::Getd3dDevice()->CreateBuffer(&bd, &sd, pVertexBuffer);
		return Error(ErrorType::Success, std::string(__FUNCTION__));
	}
	Error DX11Buffer::CreateIndexBuffer(const void* indices, unsigned int size, unsigned int stride, ID3D11Buffer** pIndexBuffer)
	{
		if (!indices)
			return Error(ErrorType::NullError, std::string(__FUNCTION__));
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.ByteWidth = size;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = indices;
		RendererBase::Getd3dDevice()->CreateBuffer(&bd, &sd, pIndexBuffer);
		return Error(ErrorType::Success, std::string(__FUNCTION__));
	}
	
}