#pragma once
#include "Pistachio/Core.h"
#include "../RendererBase.h"
#include "../Buffer.h"

namespace Pistachio {

	class DX11Buffer {
	public:
		static ID3D11Buffer* CreateVertexBuffer(const void* vertices, unsigned int size, unsigned int Stride);
		static ID3D11Buffer* CreateIndexBuffer(const void* indices, unsigned int size, unsigned int Stride);
	};
}