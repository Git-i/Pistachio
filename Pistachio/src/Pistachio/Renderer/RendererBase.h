#pragma once
#include "Pistachio/Core.h"
#include "Buffer.h"
//#include "DirectX11/DX11RendererBase.h"
#include "../Core/Instance.h"
namespace Pistachio {
	enum class CullMode {
		None, Front, Back
	};
	enum class PrimitiveTopology
	{
		TriangleList, LineList, LineStrip, Points, TriangleStrip
	};
	enum class DepthStencilOp {
		Less, Less_Equal
	};
	class PISTACHIO_API RendererBase
	{
	public:
		static bool IsDeviceNull;
		static void Shutdown();
		static void ChangeViewport(int width, int height, int x=0, int y=0);
		static void ClearTarget();
		static void CreateTarget();
		static void ClearView();
		static void Resize(int width, int height);
		static void SetPrimitiveTopology(PrimitiveTopology Topology);
		static void SetClearColor(float r, float g, float b, float a);
		static void DrawIndexed(const Buffer& buffer, unsigned int indexCount=0);
		static void SetCullMode(CullMode cullmode);
		static void EnableShadowMapRasetrizerState();
		static void SetDepthStencilOp(DepthStencilOp op);
		static void BindMainTarget();
		#ifdef PT_PLATFORM_WINDOWS
			static bool Init(HWND hwnd);
		#endif
		static RHI::Device* Getd3dDevice() {  return device; }
		static RHI::CommandList* GetMainCommandList() { return mainCommandList; }
		static RHI::SwapChain* GetSwapChain() { return swapChain; }
		//static ID3D11RenderTargetView* GetmainRenderTargetView() { return g_mainRenderTargetView.Get(); }
		//static ID3D11DepthStencilView* GetDepthStencilView(){ return pDSV.Get(); }
	private:
		static RHI::Device* device;
		static RHI::GraphicsCommandList* mainCommandList;
		static RHI::CommandAllocator* commandAllocators[3];
		static RHI::Instance* instance;
		static RHI::SwapChain* swapChain;
		static RHI::CommandQueue* directQueue;
		static RHI::Texture* backBufferTextures[2]; //todo: tripebuffering support
		static RHI::DescriptorHeap* rtvHeap;
		static std::uint64_t fence_vals[3]; //managing sync across allocators
		static RHI::Fence* mainFence;
		static FLOAT m_ClearColor[4];
	};
}


