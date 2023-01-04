#pragma once
#include "Pistachio/Core.h"
#include "../RendererBase.h"
#include "../Buffer.h"
#include "../../Core/Error.h"
namespace Pistachio {

	class DX11Buffer {
	public:
		static Error CreateVertexBuffer(const void* vertices, unsigned int size, unsigned int Stride, ID3D11Buffer** pVB);
		static Error CreateIndexBuffer(const void* indices, unsigned int size, unsigned int Stride, ID3D11Buffer** pVB);
	};
}