#pragma once
#include "../RendererBase.h"
#include "../Shader.h"
class DX11Shader {
public:
	static ID3D11VertexShader* CreateVertexShader(const wchar_t* VertexSource, ID3DBlob** pBlob);
	static ID3D11PixelShader* CreatePixelShader(const wchar_t* FragmentSource, ID3DBlob** pBlob);
	static ID3D11InputLayout* CreateInputLayout(Pistachio::BufferLayout* layout, int nAttribures, void* v, SIZE_T s);
};