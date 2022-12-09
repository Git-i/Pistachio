#include "ptpch.h"
#include "DX11Shader.h"

static DXGI_FORMAT DXGIFormat(Pistachio::BufferLayoutFormat format)
{
	switch (format)
	{
	case Pistachio::BufferLayoutFormat::FLOAT4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case Pistachio::BufferLayoutFormat::UINT4: return DXGI_FORMAT_R32G32B32A32_UINT;
	case Pistachio::BufferLayoutFormat::INT4: return  DXGI_FORMAT_R32G32B32A32_SINT;
	case Pistachio::BufferLayoutFormat::FLOAT3: return DXGI_FORMAT_R32G32B32_FLOAT;
	case Pistachio::BufferLayoutFormat::UINT3: return DXGI_FORMAT_R32G32B32_UINT;
	case Pistachio::BufferLayoutFormat::INT3: return  DXGI_FORMAT_R32G32B32_SINT;
	case Pistachio::BufferLayoutFormat::FLOAT2: return DXGI_FORMAT_R32G32_FLOAT;
	case Pistachio::BufferLayoutFormat::UINT2: return DXGI_FORMAT_R32G32_UINT;
	case Pistachio::BufferLayoutFormat::INT2: return DXGI_FORMAT_R32G32_SINT;
	case Pistachio::BufferLayoutFormat::FLOAT: return DXGI_FORMAT_R32_FLOAT;
	case Pistachio::BufferLayoutFormat::UINT: return DXGI_FORMAT_R32_UINT;
	case Pistachio::BufferLayoutFormat::INT: return DXGI_FORMAT_R32_SINT;
	default:
		break;
	}
	return DXGI_FORMAT_UNKNOWN;
}

ID3D11VertexShader* DX11Shader::CreateVertexShader(const wchar_t* VertexSource, ID3DBlob** pBlob)
{
	ID3D11VertexShader* pVertexShader = NULL;
	D3DReadFileToBlob(VertexSource, pBlob);
	Pistachio::RendererBase::Getd3dDevice()->CreateVertexShader((*pBlob)->GetBufferPointer(), (*pBlob)->GetBufferSize(), nullptr, &pVertexShader);
	return pVertexShader;
}

ID3D11PixelShader* DX11Shader::CreatePixelShader(const wchar_t* FragmentSource, ID3DBlob** pBlob)
{
	ID3D11PixelShader* pPixelShader = NULL;
	D3DReadFileToBlob(FragmentSource, pBlob);
	Pistachio::RendererBase::Getd3dDevice()->CreatePixelShader((*pBlob)->GetBufferPointer(), (*pBlob)->GetBufferSize(), nullptr, &pPixelShader);
	return pPixelShader;
}

ID3D11InputLayout* DX11Shader::CreateInputLayout(Pistachio::BufferLayout* layout, int nAttribures, void* v, SIZE_T s)
{
	ID3D11InputLayout* pInputLayout = nullptr;
	std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
	ied.reserve(nAttribures);
	for (int i = 0; i < nAttribures; i++) {
		ied.push_back(D3D11_INPUT_ELEMENT_DESC{ layout[i].Name, 0, DXGIFormat(layout[i].Format), 0, layout[i].Offset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
	}
	Pistachio::RendererBase::Getd3dDevice()->CreateInputLayout(&ied.front(), nAttribures, v, s, &pInputLayout);
	Pistachio::RendererBase::Getd3dDeviceContext()->IASetInputLayout(pInputLayout);
	return pInputLayout;
}

void DX11Shader::CreateConstantBuffer(Pistachio::ConstantBuffer& cb, ID3D11Device* device, ID3D11DeviceContext* context)
{
	ID3D11Buffer* constBuffer;
	D3D11_BUFFER_DESC cbd = {};
	cbd.ByteWidth = sizeof(cb);
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0;
	cbd.StructureByteStride = sizeof(Pistachio::ConstantBuffer);
	D3D11_SUBRESOURCE_DATA sd;
	sd.pSysMem = &cb;
	device->CreateBuffer(&cbd, &sd, &constBuffer);
	context->VSSetConstantBuffers(0, 1, &constBuffer);
}
