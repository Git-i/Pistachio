#pragma once
#include "../RendererBase.h"
#include "../Shader.h"
namespace Pistachio {
	class DX11Shader {
	public:
		static Error CreateVertexShader(const wchar_t* VertexSource, ID3DBlob** pBlob, ID3D11VertexShader** pVertexShader);
		static Error CreatePixelShader(const wchar_t* FragmentSource, ID3DBlob** pBlob, ID3D11PixelShader** pPixelShader);
		static Error CreateInputLayout(Pistachio::BufferLayout* layout, int nAttribures, void* v, SIZE_T s, ID3D11InputLayout** pInputLayout);
		static void CreateConstantBuffer(const void* cb, int size, ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Buffer** constBuffer);
	};
}