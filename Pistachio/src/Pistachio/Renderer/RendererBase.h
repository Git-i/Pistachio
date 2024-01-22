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
		static void EndFrame();
		static void ChangeViewport(int width, int height, int x=0, int y=0);
		static void ClearTarget();
		static void CreateTarget();
		static void ClearView();
		static void Resize(int width, int height);
		static void PushBufferUpdate(RHI::Buffer* buffer, uint32_t offsetFromBufferStart,const void* data, uint32_t size);
		static void PushTextureUpdate(RHI::Texture* texture, uint32_t imgByteSize,const void* data,RHI::SubResourceRange* range, RHI::Extent3D imageExtent, RHI::Offset3D imageOffset);
		static void CreateDescriptorSet(RHI::DescriptorSet**, RHI::DescriptorSetLayout* layout);
		static void FlushStagingBuffer();
		static void SetPrimitiveTopology(PrimitiveTopology Topology);
		static void SetClearColor(float r, float g, float b, float a);
		static void DrawIndexed(uint32_t indexCount);
		static void SetCullMode(CullMode cullmode);
		static void EnableShadowMapRasetrizerState();
		static void SetDepthStencilOp(DepthStencilOp op);
		static void BindMainTarget();
		#ifdef PT_PLATFORM_WINDOWS
			static bool Init(HWND hwnd);
		#endif
		static RHI::Device* Getd3dDevice() {  return device; }
		static RHI::GraphicsCommandList* GetMainCommandList() { return mainCommandList; }
		static RHI::SwapChain* GetSwapChain() { return swapChain; }
		//static ID3D11RenderTargetView* GetmainRenderTargetView() { return g_mainRenderTargetView.Get(); }
		//static ID3D11DepthStencilView* GetDepthStencilView(){ return pDSV.Get(); }
	private:
		static RHI::Device* device;
		static RHI::GraphicsCommandList* mainCommandList;
		static RHI::GraphicsCommandList* stagingCommandList;
		// using one of the frame's allocator would mean that we might reset the staging command list
		// thereby limiting the ability to queue updates effectively
		static RHI::CommandAllocator* stagingCommandAllocator; 
		static RHI::CommandAllocator* commandAllocators[3];
		static RHI::Instance* instance;
		static RHI::SwapChain* swapChain;
		static RHI::CommandQueue* directQueue;
		static RHI::Texture* backBufferTextures[2]; //todo: tripebuffering support
		static RHI::DescriptorHeap* rtvHeap;
		static std::uint64_t fence_vals[3]; //managing sync across allocators
		static std::uint64_t currentFenceVal; //managing sync across allocators
		static RHI::Fence* mainFence;
		static RHI::DescriptorHeap* heap;
		//Staging buffer to manage GPU resource updates, default size probably 2mb
		static RHI::Buffer* stagingBuffer;
		//because staging buffer updates wont happen immediately, we need the number of used bytes
		//staging buffer size will probably never cross 4gb so no need for uint64
		static uint32_t staginBufferPortionUsed;
		static uint32_t stagingBufferSize;
		static bool outstandingResourceUpdate;
		static uint32_t currentFrameIndex;
		static uint32_t currentRTVindex;
		static FLOAT m_ClearColor[4];
	};
}


