#include "ptpch.h"
#include "RendererBase.h"
#include "Pistachio/Core/Log.h"
#include "Pistachio/Core/Window.h"
#include "Pistachio/Core/Error.h"

#pragma comment(lib, "RenderHardwareInterface.lib")
static RHI::Device*              device;
static RHI::GraphicsCommandList* mainCommandList;
static RHI::CommandAllocator*    commandAllocators[3];
static RHI::Instance*            instance;
static RHI::SwapChain*           swapChain;
static RHI::CommandQueue*        directQueue;
static RHI::Texture*             backBufferTextures[2]; //todo: tripebuffering support
static RHI::DescriptorHeap*      rtvHeap;
static std::uint64_t             fence_vals[3]; //managing sync across allocators
static RHI::Fence*               mainFence;
static FLOAT                     m_ClearColor[4];
namespace Pistachio {
	//General Pistachio RendererBase API calls
	static D3D11_PRIMITIVE_TOPOLOGY DX11Topology(PrimitiveTopology Topology)
	{
		switch (Topology)
		{
		case Pistachio::PrimitiveTopology::TriangleList: return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case Pistachio::PrimitiveTopology::LineList: return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case Pistachio::PrimitiveTopology::LineStrip: return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case Pistachio::PrimitiveTopology::Points: return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		case Pistachio::PrimitiveTopology::TriangleStrip: return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		default:
			return D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
			break;
		}
	}
	void RendererBase::Shutdown()
	{
	}
	bool RendererBase::Init(HWND hwnd)
	{
		PT_PROFILE_FUNCTION();
		RHICreateInstance(&instance);
		//todo implement device selection
		RHI::PhysicalDevice* physicalDevice;
		RHI::PhysicalDeviceDesc physicalDeviceDesc;
		instance->GetPhysicalDevice(0, &physicalDevice);
		physicalDevice->GetDesc(&physicalDeviceDesc);
		std::wcout << physicalDeviceDesc.Description << std::endl;

		RHI::Surface surface;

		RHI::CommandQueueDesc commandQueueDesc = {};
		commandQueueDesc.CommandListType = RHI::CommandListType::Direct; // 1 direct cmd queue for now
		commandQueueDesc.Priority = 1.f;//only really used in vulkan

		RHICreateDevice(physicalDevice, &commandQueueDesc, 1, &directQueue, &device);
		//todo handle multiplatform surface creation
		surface.InitWin32(hwnd, instance->ID);

		unsigned int height = ((WindowData*)GetWindowDataPtr())->height;
		unsigned int width = ((WindowData*)GetWindowDataPtr())->width;
		RHI::SwapChainDesc sDesc;
		sDesc.BufferCount = 2; //todo probably allow triple buffering
		sDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // abstract this away from dxgi todo
		sDesc.Flags = 0;
		sDesc.Height = height;
		sDesc.Width = width;
		sDesc.OutputSurface = surface;
		sDesc.RefreshRate = {60,1};
		sDesc.SampleCount = 1; //disable multisampling for now, because RHI doesnt fully support it
		sDesc.SampleQuality = 0;
		sDesc.SwapChainFormat = RHI::Format::B8G8R8A8_UNORM;//todo add functionality to get supported formats in the RHI
		sDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // todo anbstract this away from dxgi
		sDesc.Windowed = true;
		instance->CreateSwapChain(&sDesc, physicalDevice, device, directQueue, &swapChain);
		device->GetSwapChainImage(swapChain, 0, &backBufferTextures[0]);
		device->GetSwapChainImage(swapChain, 1, &backBufferTextures[1]);

		RHI::PoolSize pSize;
		pSize.numDescriptors = 2;
		pSize.type = RHI::DescriptorType::RTV;
		RHI::DescriptorHeapDesc rtvHeapHesc;
		rtvHeapHesc.maxDescriptorSets = 1;
		rtvHeapHesc.numPoolSizes = 1;
		rtvHeapHesc.poolSizes = &pSize;
		//create render target views
		device->CreateDescriptorHeap(&rtvHeapHesc, &rtvHeap);
		// allocators are handled with a "frames in flight approach"
		device->CreateCommandAllocators(RHI::CommandListType::Direct, 3, commandAllocators);

		//create a main command list for now, multithreading will come later
		device->CreateCommandList(RHI::CommandListType::Direct, commandAllocators[0], &mainCommandList);
		device->CreateFence(&mainFence, 0);

		// todo find a pso handling strategy, especially for custom shaders.
		// since every pso can only hold a single shader set, probably have a bunch of
		// must have pso's for every shader, where they all create it with thier own programs
		// we can also implement custom pso's for certain shaders?? 
		// for some that have nice requirements like line rendering (and depth off??) and different comparison functions
		// meaning we'll have to have a set of static samplers globally in the rendererbase??
		return 0;
	}
	void RendererBase::ClearTarget()
	{
		//todo
	}
	void RendererBase::ChangeViewport(int width, int height, int x, int y)
	{
		RHI::Viewport vp;
		vp.width = width;
		vp.height = height;
		vp.minDepth = 0;
		vp.maxDepth = 1;
		vp.x = x;
		vp.y = y;
		mainCommandList->SetViewports(1, &vp);
	}
	void RendererBase::CreateTarget()
	{
		//todo
	}
	void RendererBase::ClearView()
	{
		//todo remove and replace with Begin/End Rendering???
	}

	void RendererBase::Resize(int width, int height)
	{
		//todo implement
	}

	void RendererBase::SetPrimitiveTopology(PrimitiveTopology Topology)
	{
		// todo replace with pso system??
	}

	void RendererBase::SetClearColor(float r, float g, float b, float a)
	{
		m_ClearColor[0] = r;
		m_ClearColor[1] = g;
		m_ClearColor[2] = b;
		m_ClearColor[3] = a;
	}

	void RendererBase::DrawIndexed(const Buffer& buffer, unsigned int indexCount)
	{
		PT_PROFILE_FUNCTION()
		unsigned int count = indexCount ? indexCount : buffer.ib->GetCount();
		buffer.Bind();
		mainCommandList->DrawIndexed(count,1, 0, 0,0);
	}

	void RendererBase::SetCullMode(CullMode cullmode)
	{
		//todo replace with pso system
	}
	void RendererBase::EnableShadowMapRasetrizerState()
	{
		//todo replace with pso system
	}
	void RendererBase::SetDepthStencilOp(DepthStencilOp op)
	{
		//todo replace with pso system
	}
	void RendererBase::BindMainTarget()
	{
		//todo replace with begine/end calls??
	}
}
