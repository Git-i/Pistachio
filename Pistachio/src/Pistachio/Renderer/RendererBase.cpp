#include "ptpch.h"
#include "RendererBase.h"
#include "Pistachio/Core/Log.h"
#include "Pistachio/Core/Window.h"
#include "Pistachio/Core/Error.h"

#pragma comment(lib, "RenderHardwareInterface.lib")
RHI::Device*              Pistachio::RendererBase::device;
RHI::GraphicsCommandList* Pistachio::RendererBase::mainCommandList;
RHI::GraphicsCommandList* Pistachio::RendererBase::stagingCommandList;
RHI::CommandAllocator*    Pistachio::RendererBase::stagingCommandAllocator;
RHI::CommandAllocator*    Pistachio::RendererBase::commandAllocators[3];
RHI::Instance*            Pistachio::RendererBase::instance;
RHI::SwapChain*           Pistachio::RendererBase::swapChain;
RHI::CommandQueue*        Pistachio::RendererBase::directQueue;
RHI::Texture*             Pistachio::RendererBase::backBufferTextures[2]; //todo: tripebuffering support
RHI::DescriptorHeap*      Pistachio::RendererBase::rtvHeap;
RHI::DescriptorHeap*      Pistachio::RendererBase::dsvHeap;
std::uint64_t             Pistachio::RendererBase::fence_vals[3]; //managing sync across allocators
std::uint64_t             Pistachio::RendererBase::currentFenceVal=0; //managing sync across allocators
RHI::Fence*               Pistachio::RendererBase::mainFence;
RHI::DescriptorHeap*      Pistachio::RendererBase::heap;
RHI::Buffer*              Pistachio::RendererBase::stagingBuffer;
RHI::Texture*			  Pistachio::RendererBase::depthTexture;
uint32_t                  Pistachio::RendererBase::staginBufferPortionUsed = 0;
uint32_t                  Pistachio::RendererBase::stagingBufferSize;
bool                      Pistachio::RendererBase::outstandingResourceUpdate = 0;
uint32_t                  Pistachio::RendererBase::currentFrameIndex =0;
uint32_t                  Pistachio::RendererBase::currentRTVindex =0;
FLOAT                     Pistachio::RendererBase::m_ClearColor[4];
namespace Pistachio {
	
	void RendererBase::Shutdown()
	{
	}
	void RendererBase::EndFrame()
	{
		RHI::TextureMemoryBarrier barr;
		barr.oldLayout = (RHI::ResourceLayout::COLOR_ATTACHMENT_OPTIMAL);
		barr.newLayout = (RHI::ResourceLayout::PRESENT);
		barr.AccessFlagsBefore = (RHI::ResourceAcessFlags::COLOR_ATTACHMENT_WRITE);
		barr.AccessFlagsAfter = (RHI::ResourceAcessFlags::NONE);
		barr.subresourceRange.imageAspect = RHI::Aspect::COLOR_BIT,
		barr.subresourceRange.IndexOrFirstMipLevel = 0,
		barr.subresourceRange.NumMipLevels = 1,
		barr.subresourceRange.FirstArraySlice = 0,
		barr.subresourceRange.NumArraySlices = 1,
		barr.texture = backBufferTextures[currentRTVindex];
		mainCommandList->PipelineBarrier(RHI::PipelineStage::COLOR_ATTACHMENT_OUTPUT_BIT, RHI::PipelineStage::BOTTOM_OF_PIPE_BIT, 0, 0, 1, &barr);
		//execute main command list
		mainCommandList->End();
		fence_vals[currentFrameIndex] = ++currentFenceVal;
		directQueue->ExecuteCommandLists(&mainCommandList->ID, 1);
		directQueue->SignalFence(mainFence, currentFenceVal); //todo add fence signaling together with queue
		swapChain->Present(currentRTVindex);
		currentRTVindex = (currentRTVindex + 1) % 2;
		currentFrameIndex = (currentFrameIndex + 1) % 3;
		//prep for next frame
		mainFence->Wait(fence_vals[currentFrameIndex]);
		commandAllocators[currentFrameIndex]->Reset();
		mainCommandList->Begin(commandAllocators[currentFrameIndex]);
		barr.AccessFlagsBefore = RHI::ResourceAcessFlags::NONE;
		barr.AccessFlagsAfter = RHI::ResourceAcessFlags::COLOR_ATTACHMENT_WRITE;
		barr.oldLayout = RHI::ResourceLayout::UNDEFINED;
		barr.newLayout = RHI::ResourceLayout::COLOR_ATTACHMENT_OPTIMAL;
		barr.texture = backBufferTextures[currentRTVindex];
		mainCommandList->PipelineBarrier(RHI::PipelineStage::TOP_OF_PIPE_BIT, RHI::PipelineStage::COLOR_ATTACHMENT_OUTPUT_BIT, 0, nullptr, 1, &barr);
		//wait for the the cmd allocator to be done
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

		PT_CORE_INFO("Creating Device");
		RHICreateDevice(physicalDevice, &commandQueueDesc, 1, &directQueue, instance->ID ,&device );
		PT_CORE_INFO("Device Created ID:{0} Internal_ID:{1}", (void*)device, (void*)device->ID);
		//todo handle multiplatform surface creation
		PT_CORE_INFO("Creating Surface foe Win32");
		surface.InitWin32(hwnd, instance->ID);
		PT_CORE_INFO("Surface Initialized");
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
		PT_CORE_INFO("Creating Swapchain");
		instance->CreateSwapChain(&sDesc, physicalDevice, device, directQueue, &swapChain);
		device->GetSwapChainImage(swapChain, 0, &backBufferTextures[0]);
		device->GetSwapChainImage(swapChain, 1, &backBufferTextures[1]);
		PT_CORE_INFO("Swapchain Created ID:{0} Internal_ID:{1}", (void*)swapChain, (void*)swapChain->ID);

		RHI::PoolSize pSize;
		pSize.numDescriptors = 2;
		pSize.type = RHI::DescriptorType::RTV;
		RHI::DescriptorHeapDesc rtvHeapHesc;
		rtvHeapHesc.maxDescriptorSets = 1;
		rtvHeapHesc.numPoolSizes = 1;
		rtvHeapHesc.poolSizes = &pSize;
	
		//create render target views
		device->CreateDescriptorHeap(&rtvHeapHesc, &rtvHeap);
		PT_CORE_INFO("Created RTV descriptor heaps");
		for (int i = 0; i < 2; i++)
		{
			RHI::CPU_HANDLE handle;
			handle.val = rtvHeap->GetCpuHandle().val + (i * device->GetDescriptorHeapIncrementSize(RHI::DescriptorClass::RTV));
			device->CreateRenderTargetView(backBufferTextures[i], nullptr, handle);
		}
		PT_CORE_INFO("Created Tender Target Views");
		RHI::ClearValue depthVal;
		depthVal.depthStencilValue.depth = 1;
		depthVal.depthStencilValue.stecnil = 0;
		depthVal.format = RHI::Format::D32_FLOAT;
		RHI::TextureDesc depthTextureDsc;
		depthTextureDsc.depthOrArraySize = 1;
		depthTextureDsc.format = RHI::Format::D32_FLOAT;
		depthTextureDsc.height = height;
		depthTextureDsc.mipLevels = 1;
		depthTextureDsc.mode = RHI::TextureTilingMode::Optimal;
		depthTextureDsc.sampleCount = 1;
		depthTextureDsc.type = RHI::TextureType::Texture2D;
		depthTextureDsc.usage = RHI::TextureUsage::DepthStencilAttachment;
		depthTextureDsc.width = width;
		depthTextureDsc.optimizedClearValue = &depthVal;

		PT_CORE_INFO("Creating DSV Heap");
		pSize.type = RHI::DescriptorType::DSV;
		pSize.numDescriptors = 1;
		device->CreateDescriptorHeap(&rtvHeapHesc, &dsvHeap);
		PT_CORE_INFO("Created DSV Heap");

		PT_CORE_INFO("Creating Default depth texture");
		RHI::AutomaticAllocationInfo DAinfo;
		DAinfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		device->CreateTexture(&depthTextureDsc, &depthTexture,0,0,&DAinfo, 0, RHI::ResourceType::Automatic);
		PT_CORE_INFO("Created Default depth texture");

		device->CreateDepthStencilView(depthTexture, nullptr, dsvHeap->GetCpuHandle());
		// allocators are handled with a "frames in flight approach"
		RESULT res = device->CreateCommandAllocators(RHI::CommandListType::Direct, 3, commandAllocators);
		if (res != 0) PT_CORE_INFO("Main allocator creation failed with code :{0}", res);
		else PT_CORE_INFO("Created main command allocators");
		device->CreateCommandAllocators(RHI::CommandListType::Direct, 1, &stagingCommandAllocator);
		PT_CORE_INFO("Created staging command allocator(s)");
		device->CreateCommandList(RHI::CommandListType::Direct, stagingCommandAllocator, &stagingCommandList);
		PT_CORE_INFO("Created staging command list");
		stagingCommandList->Begin(stagingCommandAllocator);
		PT_CORE_INFO("Began staging command list");
		//create a main command list for now, multithreading will come later
		device->CreateCommandList(RHI::CommandListType::Direct, commandAllocators[0], &mainCommandList);
		PT_CORE_INFO("Created main command list");
		device->CreateFence(&mainFence, 0);
		PT_CORE_INFO("Created main fence");


		RHI::PoolSize HeapSizes[2];
		
		HeapSizes[0].numDescriptors = 5;
		HeapSizes[0].type = RHI::DescriptorType::ConstantBuffer;

		HeapSizes[1].numDescriptors = 5;
		HeapSizes[1].type = RHI::DescriptorType::SampledTexture;

		RHI::DescriptorHeapDesc DHdesc;
		DHdesc.maxDescriptorSets = 10;
		DHdesc.numPoolSizes = 1;
		DHdesc.poolSizes = HeapSizes;
		device->CreateDescriptorHeap(&DHdesc, &heap);
		PT_CORE_INFO("Created main descriptor heap");
		

		//initialize the staing buffer
		stagingBufferSize = 2 * 1024 * 1024;//2mb stagin buffer ???
		RHI::BufferDesc stagingBufferDesc;
		stagingBufferDesc.size = stagingBufferSize;
		stagingBufferDesc.usage = RHI::BufferUsage::CopySrc;
		RHI::AutomaticAllocationInfo allocInfo;
		allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::Sequential;
		device->CreateBuffer(&stagingBufferDesc, &stagingBuffer, nullptr, nullptr, &allocInfo, 0, RHI::ResourceType::Automatic);

		//prep for rendering
		mainCommandList->Begin(commandAllocators[0]);
		RHI::TextureMemoryBarrier barr;
		barr.AccessFlagsBefore = RHI::ResourceAcessFlags::NONE;
		barr.AccessFlagsAfter = RHI::ResourceAcessFlags::COLOR_ATTACHMENT_WRITE;
		barr.oldLayout = RHI::ResourceLayout::UNDEFINED;
		barr.newLayout = RHI::ResourceLayout::COLOR_ATTACHMENT_OPTIMAL;
		barr.subresourceRange.imageAspect = RHI::Aspect::COLOR_BIT;
		barr.subresourceRange.IndexOrFirstMipLevel = 0;
		barr.subresourceRange.NumMipLevels = 1;
		barr.subresourceRange.FirstArraySlice = 0;
		barr.subresourceRange.NumArraySlices = 1;
		barr.texture = backBufferTextures[0];

		mainCommandList->PipelineBarrier(RHI::PipelineStage::TOP_OF_PIPE_BIT, RHI::PipelineStage::COLOR_ATTACHMENT_OUTPUT_BIT, 0, nullptr, 1, &barr);
		PT_CORE_INFO("done initializing rhi");
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

	void RendererBase::PushBufferUpdate(RHI::Buffer* buffer, uint32_t offsetFromBufferStart, const void* data, uint32_t size)
	{
		//check if we have enough space to queue the write
		if ((stagingBufferSize - staginBufferPortionUsed) < size)
		{
			FlushStagingBuffer();
		}
		//check if resource is bigger than the entire buffer
		if(size > stagingBufferSize)
		{
			return;
			//split updates into portions and return from function
			// or make a larger buffer temporarily, or better expand the staging buffer
		}
		void* stagingBufferPointer;
		stagingBuffer->Map(&stagingBufferPointer);
		stagingBufferPointer = ((std::uint8_t*)stagingBufferPointer) + staginBufferPortionUsed; //offset by the amount of bytes already used
		memcpy(stagingBufferPointer, data, size);
		stagingBuffer->UnMap();
		stagingCommandList->CopyBufferRegion(staginBufferPortionUsed, offsetFromBufferStart, size, stagingBuffer, buffer);
		staginBufferPortionUsed += size;
		outstandingResourceUpdate = true;
	}

	void RendererBase::PushTextureUpdate(RHI::Texture* texture, uint32_t size ,const void* data,RHI::SubResourceRange* range, RHI::Extent3D imageExtent, RHI::Offset3D imageOffset)
	{
		PT_CORE_ASSERT((size % (imageExtent.width * imageExtent.height)) == 0);
		//check if we have enough space to queue the write
		if ((stagingBufferSize - staginBufferPortionUsed) < size)
		{
			FlushStagingBuffer();
		}
		//check if resource is bigger than the entire buffer
		if (size > stagingBufferSize)
		{
			//split updates into portions and return from function
			// or make a larger buffer temporarily, or better expand the staging buffer
			return;
		}
		void* stagingBufferPointer;
		stagingBuffer->Map(&stagingBufferPointer);
		stagingBufferPointer = ((std::uint8_t*)stagingBufferPointer) + staginBufferPortionUsed; //offset by the amount of bytes already used
		memcpy(stagingBufferPointer, data, size);
		stagingBuffer->UnMap();
		stagingCommandList->CopyBufferToImage(staginBufferPortionUsed, *range, imageOffset, imageExtent, stagingBuffer, texture);
		staginBufferPortionUsed += size;
		outstandingResourceUpdate = true;
	}

	void RendererBase::CreateDescriptorSet(RHI::DescriptorSet** set, RHI::DescriptorSetLayout* layout)
	{
		device->CreateDescriptorSets(heap, 1, layout, set);
	}

	void RendererBase::FlushStagingBuffer()
	{
		stagingCommandList->End();
		directQueue->ExecuteCommandLists(&stagingCommandList->ID, 1); //todo look into dedicated transfer queue ??
		directQueue->SignalFence(mainFence, 100); // 100 used here is an arbitrary value, hopefully we dont have to flush before frame 1?
		mainFence->Wait(100); // consider doing this async perhaps;
		directQueue->SignalFence(mainFence, currentFenceVal);
		stagingCommandAllocator->Reset();
		stagingCommandList->Begin(stagingCommandAllocator);
		staginBufferPortionUsed = 0;
		outstandingResourceUpdate = false;
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

	void RendererBase::DrawIndexed(uint32_t indexCount)
	{
		mainCommandList->DrawIndexed(indexCount, 1, 0, 0, 0);
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
	RHI::Device* RendererBase::Getd3dDevice()
	{
		return device; 
	}
	RHI::GraphicsCommandList* Pistachio::RendererBase::GetMainCommandList()
	{
		return mainCommandList; 
	}
	RHI::SwapChain*      RendererBase::GetSwapChain() { return swapChain; }
	RHI::DescriptorHeap* RendererBase::GetRTVDescriptorHeap() { return rtvHeap; };
	uint32_t             RendererBase::GetCurrentRTVIndex() { return currentRTVindex; }
	RHI::DescriptorHeap* RendererBase::GetDSVDescriptorHeap(){return dsvHeap;}
}
