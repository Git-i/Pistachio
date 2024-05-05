#pragma once
#include "Pistachio/Core.h"
#include "Buffer.h"
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
	struct RTVHandle
	{
		uint32_t heapIndex;
		uint32_t heapOffset;
	};
	struct DSVHandle
	{
		uint32_t heapIndex;
		uint32_t heapOffset;
	};
	struct SamplerHandle
	{
		uint32_t heapIndex;
		uint32_t heapOffset;
	};
	struct TrackedDescriptorHeap
	{
		RHI::DescriptorHeap* heap;
		uint32_t sizeLeft = 0;
		uint32_t freeOffset = 0;
	};
	
	class PISTACHIO_API RendererBase
	{
	public:
		struct InitOptions
		{
			bool headless;
			RHI::LUID luid;
			bool exportTexture;
		};
		static bool IsDeviceNull;
		static void Shutdown();
		static void EndFrame();
		static void ClearTarget();
		static void CreateTarget();
		static RHI::API GetAPI();
		static RTVHandle CreateRenderTargetView(RHI::Texture* texture, RHI::RenderTargetViewDesc* viewDesc);
		static DSVHandle CreateDepthStencilView(RHI::Texture* texture, RHI::DepthStencilViewDesc* viewDesc);
		static SamplerHandle CreateSampler(RHI::SamplerDesc* viewDesc);
		static void DestroyRenderTargetView(RTVHandle handle);
		static void DestroyDepthStencilView(DSVHandle handle);
		static void DestroySampler(SamplerHandle handle);
		static RHI::CPU_HANDLE GetCPUHandle(RTVHandle handle);
		static RHI::CPU_HANDLE GetCPUHandle(DSVHandle handle);
		static RHI::CPU_HANDLE GetCPUHandle(SamplerHandle handle);
		static void ChangeViewport(int width, int height, int x=0, int y=0);
		static void ClearView();
		static void Resize(int width, int height);
		static void PushBufferUpdate(RHI::Buffer* buffer, uint32_t offsetFromBufferStart,const void* data, uint32_t size);
		static void PushTextureUpdate(RHI::Texture* texture, uint32_t imgByteSize,const void* data,RHI::SubResourceRange* range, RHI::Extent3D imageExtent, RHI::Offset3D imageOffset,RHI::Format format);
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
		static bool Init(HWND hwnd, InitOptions& options);
		#endif
		static RHI::Device* Getd3dDevice();
		static RHI::GraphicsCommandList* GetMainCommandList();
		static RHI::SwapChain* GetSwapChain();
		static RHI::DescriptorHeap* GetRTVDescriptorHeap();
		static RHI::DescriptorHeap* GetDSVDescriptorHeap();
		static RHI::DescriptorHeap* GetMainDescriptorHeap();
		static RHI::Texture* GetBackBufferTexture(uint32_t index);
		static RHI::Texture* GetDefaultDepthTexture();
		static uint32_t GetCurrentRTVIndex();
		static uint32_t GetCurrentFrameIndex();
		static const constexpr uint32_t numFramesInFlight = 3;
		//static ID3D11RenderTargetView* GetmainRenderTargetView() { return g_mainRenderTargetView.Get(); }
		//static ID3D11DepthStencilView* GetDepthStencilView(){ return pDSV.Get(); }
	private:
		friend class Renderer; //easy access to avoid fn calls
		friend class Texture2D;
		friend class Shader;
		friend class RenderGraph;
		friend class RenderTexture;
		friend class RenderCubeMap;
		friend class DepthTexture;
		friend class ShaderAsset;
		friend class ComputeShader;
		friend class FrameComposer;
		friend class Scene;
		static RHI::Device* device;
		static RHI::GraphicsCommandList* mainCommandList;
		static RHI::GraphicsCommandList* stagingCommandList;
		// using one of the frame's allocator would mean that we might reset the staging command list
		// thereby limiting the ability to queue updates effectively
		static RHI::CommandAllocator* stagingCommandAllocator; 
		static RHI::CommandAllocator* commandAllocators[3];
		static RHI::CommandAllocator* computeCommandAllocators[3];
		static RHI::Instance* instance;
		static RHI::SwapChain* swapChain;
		static RHI::CommandQueue* directQueue;
		static RHI::CommandQueue* computeQueue;
		static RHI::Texture* backBufferTextures[2]; //todo: tripebuffering support
		static RHI::Texture* depthTexture;
		static RHI::DescriptorHeap* MainRTVheap;
		static std::vector<TrackedDescriptorHeap> rtvHeaps;
		static std::vector<RTVHandle> freeRTVs;
		static std::vector<TrackedDescriptorHeap> dsvHeaps;
		static std::vector<DSVHandle> freeDSVs;
		static std::vector<TrackedDescriptorHeap> samplerHeaps;
		static std::vector<SamplerHandle> freeSamplers;
		static RHI::DescriptorHeap* dsvHeap;
		static std::uint64_t fence_vals[3]; //managing sync across allocators
		static std::uint64_t currentFenceVal; //managing sync across allocators
		static RHI::Fence* mainFence;
		static RHI::Fence* stagingFence;
		static RHI::DescriptorHeap* heap;
		//Staging buffer to manage GPU resource updates, default size probably 2mb
		static RHI::Buffer* stagingBuffer;
		static bool headless;
		//because staging buffer updates wont happen immediately, we need the number of used bytes
		//staging buffer size will probably never cross 4gb so no need for uint64
		static uint32_t staginBufferPortionUsed;
		static uint32_t stagingBufferSize;
		static bool outstandingResourceUpdate;
		static bool MQ;
		static uint32_t currentFrameIndex;
		static uint32_t currentRTVindex;
		static FLOAT m_ClearColor[4];
	};
}


