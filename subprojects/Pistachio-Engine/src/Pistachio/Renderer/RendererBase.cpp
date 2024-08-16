#include "Barrier.h"
#include "CommandList.h"
#include "Device.h"
#include "FormatsAndTypes.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Pistachio/Debug/Instrumentor.h"
#include "Pistachio/Renderer/RenderGraph.h"
#include "Ptr.h"
#include "Texture.h"
#include "TraceRHI.h"
#include "ptpch.h"
#include "RendererBase.h"
#include "Pistachio/Core/Log.h"
#include "Pistachio/Core/Window.h"
#include "Pistachio/Core/Error.h"
#include "Util/FormatUtils.h"
#include <cstdint>
#include <format>
#include <ranges>
#include <string_view>
#include <unordered_map>

RHI::Ptr<RHI::Device>              Pistachio::RendererBase::device;
RHI::Ptr<RHI::GraphicsCommandList> Pistachio::RendererBase::mainCommandList;
RHI::Ptr<RHI::GraphicsCommandList> Pistachio::RendererBase::stagingCommandList;
RHI::Ptr<RHI::CommandAllocator>    Pistachio::RendererBase::stagingCommandAllocator;
RHI::Ptr<RHI::CommandAllocator>    Pistachio::RendererBase::commandAllocators[3];
RHI::Ptr<RHI::CommandAllocator>    Pistachio::RendererBase::computeCommandAllocators[3];
RHI::Instance*            Pistachio::RendererBase::instance;
RHI::Ptr<RHI::SwapChain>           Pistachio::RendererBase::swapChain;
RHI::Ptr<RHI::CommandQueue>        Pistachio::RendererBase::directQueue;
std::vector<RHI::Ptr<RHI::Texture>>Pistachio::RendererBase::backBufferTextures; //todo: tripebuffering support
RHI::Ptr<RHI::DescriptorHeap>      Pistachio::RendererBase::MainRTVheap;
RHI::Ptr<RHI::DescriptorHeap>      Pistachio::RendererBase::dsvHeap;
Pistachio::Texture2D			   Pistachio::RendererBase::whiteTexture;
Pistachio::Texture2D			   Pistachio::RendererBase::blackTexture;
std::uint64_t             Pistachio::RendererBase::fence_vals[3]; //managing sync across allocators
std::uint64_t             Pistachio::RendererBase::currentFenceVal=0; //managing sync across allocators
RHI::Ptr<RHI::Fence>               Pistachio::RendererBase::mainFence;
RHI::Ptr<RHI::Fence>               Pistachio::RendererBase::stagingFence;
RHI::Ptr<RHI::DescriptorHeap>      Pistachio::RendererBase::heap;
RHI::Ptr<RHI::CommandQueue>		  Pistachio::RendererBase::computeQueue;
RHI::Ptr<RHI::Buffer>              Pistachio::RendererBase::stagingBuffer;
RHI::Ptr<RHI::Texture>			  Pistachio::RendererBase::depthTexture;
uint32_t                  Pistachio::RendererBase::staginBufferPortionUsed = 0;
uint32_t                  Pistachio::RendererBase::stagingBufferSize;
TraceRHI::Context		  Pistachio::RendererBase::traceRHICtx;
bool                      Pistachio::RendererBase::outstandingResourceUpdate = 0;
bool                      Pistachio::RendererBase::MQ = false;
bool					  Pistachio::RendererBase::headless;
uint32_t                  Pistachio::RendererBase::currentFrameIndex =0;
uint32_t                  Pistachio::RendererBase::currentRTVindex =0;
uint32_t				  Pistachio::RendererBase::swapCycleIndex = 0;
uint32_t				  Pistachio::RendererBase::numSwapImages = 0;
float                     Pistachio::RendererBase::m_ClearColor[4];
std::vector<Pistachio::TrackedDescriptorHeap> Pistachio::RendererBase::rtvHeaps;
std::vector<Pistachio::RTVHandle> Pistachio::RendererBase::freeRTVs;
std::vector<Pistachio::TrackedDescriptorHeap> Pistachio::RendererBase::dsvHeaps;
std::vector<Pistachio::DSVHandle> Pistachio::RendererBase::freeDSVs;
std::vector<Pistachio::TrackedDescriptorHeap> Pistachio::RendererBase::samplerHeaps;
std::vector<Pistachio::SamplerHandle> Pistachio::RendererBase::freeSamplers;
static const uint32_t STAGING_BUFFER_INITIAL_SIZE = 80 * 1024 * 1024; //todo: reduce this
namespace Pistachio {
	static uint32_t SwapImageCount(std::pair<uint32_t, uint32_t> minMax, uint32_t preferred)
	{
		PT_CORE_ASSERT(preferred >= 1);
		uint32_t chosen;
		if (minMax.second == 0) minMax.second = UINT32_MAX;
		chosen = minMax.first;
		//we opt for at least double buffering if available
		uint32_t max_available = std::min(preferred, minMax.second);
		if(chosen < max_available) chosen = max_available;
		return chosen;
	}
	void RendererBase::Shutdown()
	{
		mainFence->Wait(fence_vals[(currentFrameIndex+2)%3]);
	}
	void RendererBase::BackBufferBarrier(RHI::PipelineStage before,RHI::PipelineStage after,RHI::ResourceLayout oldLayout, RHI::ResourceLayout newLayout,
				RHI::ResourceAcessFlags prevAccess, RHI::ResourceAcessFlags currAccess)
	{
		RHI::TextureMemoryBarrier barr{};
		barr.oldLayout = oldLayout;
		barr.newLayout = newLayout;
		barr.AccessFlagsBefore = prevAccess;
		barr.AccessFlagsAfter = currAccess;
		barr.subresourceRange = 
		{	
			.imageAspect = RHI::Aspect::COLOR_BIT,
			.IndexOrFirstMipLevel = 0,
			.NumMipLevels = 1,
			.FirstArraySlice = 0,
			.NumArraySlices = 1,
		};
		barr.texture = backBufferTextures[currentRTVindex];
		barr.previousQueue = barr.nextQueue = RHI::QueueFamily::Ignored;
		mainCommandList->PipelineBarrier(before, after, {},{&barr,1});
	}
	void RendererBase::EndFrame()
	{
		PT_PROFILE_FUNCTION();
		if (!headless)
		{
			BackBufferBarrier(
				RHI::PipelineStage::TRANSFER_BIT,
				RHI::PipelineStage::BOTTOM_OF_PIPE_BIT, 

				RHI::ResourceLayout::TRANSFER_DST_OPTIMAL,
				RHI::ResourceLayout::PRESENT, 

				RHI::ResourceAcessFlags::TRANSFER_WRITE, 
				RHI::ResourceAcessFlags::NONE);
		}
		//execute main command list
		mainCommandList->End();
		directQueue->ExecuteCommandLists(&mainCommandList->ID, 1);
		fence_vals[currentFrameIndex] = ++currentFenceVal;
		directQueue->SignalFence(mainFence, currentFenceVal); //todo add fence signaling together with queue
		if(!headless)
		{
			{
				PT_PROFILE_SCOPE("SwapChain Present");
				swapChain->Present(mainFence, fence_vals[currentFrameIndex]);
				swapCycleIndex = (swapCycleIndex +1)%numSwapImages;
			}
		}
		
		//cycle frame Index
		currentFrameIndex = (currentFrameIndex + 1) % 3;
		currentRTVindex  = (currentRTVindex + 1) % numSwapImages;
		//prep for next frame
		{
			PT_PROFILE_SCOPE("Wait For Past Frame To Complete");
			mainFence->Wait(fence_vals[currentFrameIndex]);
		}
		{
			PT_PROFILE_SCOPE("Prep Command List and Allocators for Next Frame");
			commandAllocators[currentFrameIndex]->Reset();
			if(MQ) computeCommandAllocators[currentFrameIndex]->Reset();
			mainCommandList->Begin(commandAllocators[currentFrameIndex]);
		}
		if (!headless)
		{
			BackBufferBarrier(
				RHI::PipelineStage::TOP_OF_PIPE_BIT,
				RHI::PipelineStage::TRANSFER_BIT, 

				RHI::ResourceLayout::UNDEFINED,
				RHI::ResourceLayout::TRANSFER_DST_OPTIMAL, 

				RHI::ResourceAcessFlags::NONE, 
				RHI::ResourceAcessFlags::TRANSFER_WRITE);
		}
		//wait for the the cmd allocator to be done
	}
	uint32_t DeviceScore(RHI::PhysicalDevice* device)
	{
		uint32_t score = 1;
		auto desc = device->GetDesc();
		if(desc.type == RHI::DeviceType::Dedicated)
		{
			score += 5;
		}
		else if(desc.type == RHI::DeviceType::Integrated)
		{
			score += 2;
		}
		return score;
	}
	static RHI::PhysicalDevice* SelectPhysicalDevice(std::span<RHI::PhysicalDevice*> devices)
	{
		std::unordered_map<RHI::PhysicalDevice*, uint32_t> scores;
		for(auto& device: devices)
		{
			scores[device] = DeviceScore(device);
		}
		std::pair<RHI::PhysicalDevice*, uint32_t> device = {nullptr, 0};
		for(auto& score : scores)
		{
			if (score.second > device.second)
			{
				device = score;
			}
		}
		return device.first;
	}
	bool RendererBase::Init(PlatformData* pd, InitOptions& options)
	{
		PT_PROFILE_FUNCTION();
		PT_CORE_ASSERT((options.custom_instance && options.custom_device) || (!options.custom_instance && !options.custom_device));
		headless = options.headless;
		if(options.custom_instance) instance = RHI::Instance::FromNativeHandle(options.custom_instance);
		else RHICreateInstance(&instance);
		instance->SetLoggerCallback([](RHI::LogLevel l, std::string_view str)
		{
			using enum RHI::LogLevel;
			switch(l)
			{
				case Error: PT_CORE_ERROR("RHI: {0}", str);
				case Warn: PT_CORE_WARN("RHI: {0}", str);
				case Info: PT_CORE_INFO("RHI: {0}", str);
			}
		});
		//todo implement device selection
		PT_CORE_INFO("Initializing RendererBase");
		RHI::PhysicalDevice* physicalDevice;
		if(options.custom_device)
		{
			PT_CORE_INFO("Custom Device Provided");
			MQ = false; //with a custom device we only require one queue
			RHI::CommandQueueDesc desc;
			desc.commandListType = RHI::CommandListType::Direct;
			desc.Priority = 1.f;
			device = RHI::Device::FromNativeHandle(options.custom_device, options.custom_physical_device,options.custom_instance, options.indices).value();
			physicalDevice = RHI::PhysicalDevice::FromNativeHandle(options.custom_physical_device);
			directQueue = RHI::CommandQueue::FromNativeHandle(options.custom_direct_queue);
			if(options.custom_compute_queue)
			{
				MQ = true;
				computeQueue = RHI::CommandQueue::FromNativeHandle(options.custom_compute_queue);
			}
		}
		else {
			uint32_t num_devices =  instance->GetNumPhysicalDevices();
			PT_CORE_INFO("Found {0} physical devices: ", num_devices);
			std::vector<RHI::PhysicalDevice*> pdevices(num_devices);
			instance->GetAllPhysicalDevices(pdevices.data());
			physicalDevice = options.custom_fn ?  options.custom_fn(pdevices) : SelectPhysicalDevice(pdevices);
			for (auto pDevice : pdevices)
			{
				auto pDDesc = pDevice->GetDesc();
				if(options.useLuid) if (memcmp(options.luid.data, pDDesc.AdapterLuid.data, 8) == 0) physicalDevice = pDevice;
				PT_CORE_INFO("    {0}", pDDesc.Description);
			}
			
			RHI::CommandQueueDesc commandQueueDesc[2] {};
			commandQueueDesc[0].commandListType = RHI::CommandListType::Direct;
			commandQueueDesc[0].Priority = 1.f;//only really used in vulkan
			commandQueueDesc[1].commandListType = RHI::CommandListType::Compute;
			commandQueueDesc[1].Priority = 1.f;

			PT_CORE_INFO("Creating Device");
			auto flag = options.exportTexture ? RHI::DeviceCreateFlags::ShareAutomaticMemory: RHI::DeviceCreateFlags::None;
			auto [dev, queue] = RHI::Device::Create(physicalDevice, commandQueueDesc, 2, instance->ID, &MQ, flag).value();
			device = dev;
			directQueue = queue[0];
			if(MQ) computeQueue = queue[1];
			PT_CORE_INFO("Device Created ID:{0}, Physical Device used [{1}]", device->ID, physicalDevice->GetDesc().Description);
		}


		device->SetName("RHI Device");
		RHI::Surface surface;
		//todo handle multiplatform surface creation
		unsigned int height = 1280;
		unsigned int width = 720;
		if (!options.headless)
		{
			#ifdef WIN32
			PT_CORE_INFO("Creating Surface for Win32");
			surface.InitWin32(pd.hwnd, instance->ID);
			#else
			PT_CORE_INFO("Creating Surface for GLFW window");
			surface.InitGLFW(pd->window, instance->ID);
			#endif
			PT_CORE_INFO("Surface Initialized");

			height = ((WindowData*)GetWindowDataPtr())->height;
			width  = ((WindowData*)GetWindowDataPtr())->width;
			RHI::SwapChainDesc sDesc;
			sDesc.BufferCount = SwapImageCount(instance->GetSwapChainMinMaxImageCount(physicalDevice, &surface), 2);
			sDesc.Flags = 0;
			sDesc.Height = height;
			sDesc.Width = width;
			sDesc.OutputSurface = surface;
			sDesc.RefreshRate = { 60,1 };
			sDesc.SampleCount = 1; //disable multisampling for now, because RHI doesnt fully support it
			sDesc.SampleQuality = 0;
			sDesc.SwapChainFormat = RHI::Format::B8G8R8A8_UNORM;//todo add functionality to get supported formats in the RHI
			sDesc.Windowed = true;
			PT_CORE_INFO("Creating Swapchain");
			swapChain = instance->CreateSwapChain(sDesc, physicalDevice, device, directQueue).value();
			numSwapImages = sDesc.BufferCount;
			backBufferTextures.resize(numSwapImages);
			for(uint32_t i = 0; i < numSwapImages; i++)
			{
				backBufferTextures[i] = device->GetSwapChainImage(swapChain, i).value();
				PT_DEBUG_REGION(
					char name[20] = {"Back Buffer Image 1"};
					name[18] += i;
					backBufferTextures[i]->SetName(name);
				)
			}
			PT_CORE_INFO("Swapchain Created Internal_ID:{0}", swapChain->ID);
			swapChain->SetName("SwapChain");
		}

		RHI::PoolSize pSize;
		pSize.numDescriptors = 2;
		pSize.type = RHI::DescriptorType::RTV;
		RHI::DescriptorHeapDesc rtvHeapHesc;
		rtvHeapHesc.maxDescriptorSets = 1;
		rtvHeapHesc.numPoolSizes = 1;
		rtvHeapHesc.poolSizes = &pSize;

		if (!options.headless)
		{
			//create render target views
			MainRTVheap = device->CreateDescriptorHeap(rtvHeapHesc).value();
			PT_CORE_INFO("Created RTV descriptor heaps");
			for (int i = 0; i < 2; i++)
			{
				RHI::CPU_HANDLE handle;
				handle.val = MainRTVheap->GetCpuHandle().val + (i * device->GetDescriptorHeapIncrementSize(RHI::DescriptorType::RTV));
				RHI::RenderTargetViewDesc rtvDesc;
				rtvDesc.arraySlice = rtvDesc.TextureArray = rtvDesc.textureMipSlice = 0;
				rtvDesc.format = RHI::Format::B8G8R8A8_UNORM;
				device->CreateRenderTargetView(backBufferTextures[i], rtvDesc, handle);
			}
			PT_CORE_INFO("Created Tender Target Views");
		}
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
		dsvHeap = device->CreateDescriptorHeap(rtvHeapHesc).value();
		PT_CORE_INFO("Created DSV Heap");

		PT_CORE_INFO("Creating Default depth texture");
		RHI::AutomaticAllocationInfo DAinfo;
		DAinfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		depthTexture = device->CreateTexture(depthTextureDsc,0,0,&DAinfo, 0, RHI::ResourceType::Automatic).value();
		PT_DEBUG_REGION(depthTexture->SetName("Default Depth Texture"));
		PT_CORE_INFO("Created Default depth texture");
		RHI::DepthStencilViewDesc dsvDesc;
		dsvDesc.arraySlice = dsvDesc.TextureArray = dsvDesc.textureMipSlice = 0;
		dsvDesc.format = RHI::Format::D32_FLOAT;
		device->CreateDepthStencilView(depthTexture, dsvDesc, dsvHeap->GetCpuHandle());
		// allocators are handled with a "frames in flight approach"
		commandAllocators[0] = device->CreateCommandAllocator(RHI::CommandListType::Direct).value();
		commandAllocators[1] = device->CreateCommandAllocator(RHI::CommandListType::Direct).value();
		commandAllocators[2] = device->CreateCommandAllocator(RHI::CommandListType::Direct).value();
		PT_CORE_INFO("Created main command allocators");
		if(MQ)
		{
			computeCommandAllocators[0] = device->CreateCommandAllocator(RHI::CommandListType::Compute).value();
			computeCommandAllocators[1] = device->CreateCommandAllocator(RHI::CommandListType::Compute).value();
			computeCommandAllocators[2] = device->CreateCommandAllocator(RHI::CommandListType::Compute).value();
			PT_CORE_INFO("Created compute command allocators");
		}
		stagingCommandAllocator = device->CreateCommandAllocator(RHI::CommandListType::Direct).value();
		PT_CORE_INFO("Created staging command allocator(s)");
		stagingCommandList = device->CreateCommandList(RHI::CommandListType::Direct, stagingCommandAllocator).value();
		PT_DEBUG_REGION(stagingCommandList->SetName("Staging List"));
		PT_CORE_INFO("Created staging command list");
		stagingCommandList->Begin(stagingCommandAllocator);
		PT_CORE_INFO("Began staging command list");
		//create a main command list for now, multithreading will come later
		mainCommandList = device->CreateCommandList(RHI::CommandListType::Direct, commandAllocators[0]).value();
		PT_DEBUG_REGION(mainCommandList->SetName("Main Command List"));
		PT_CORE_INFO("Created main command list");
		mainFence = device->CreateFence(0).value();
		stagingFence = device->CreateFence(0).value();
		PT_CORE_INFO("Created fence(s)");

		PT_CORE_INFO("Creating Trace Context");
		traceRHICtx = TraceRHI::Context(instance, physicalDevice, device, directQueue, mainCommandList);

		RHI::PoolSize HeapSizes[3];

		HeapSizes[0].numDescriptors = 40;
		HeapSizes[0].type = RHI::DescriptorType::ConstantBuffer;

		HeapSizes[1].numDescriptors = 40;
		HeapSizes[1].type = RHI::DescriptorType::SampledTexture;

		HeapSizes[2].numDescriptors = 40;
		HeapSizes[2].type = RHI::DescriptorType::StructuredBuffer;

		RHI::DescriptorHeapDesc DHdesc;
		DHdesc.maxDescriptorSets = 120;
		DHdesc.numPoolSizes = 3;
		DHdesc.poolSizes = HeapSizes;
		heap = device->CreateDescriptorHeap(DHdesc).value();
		PT_CORE_INFO("Created main descriptor heap");


		//initialize the staing buffer
		stagingBufferSize = STAGING_BUFFER_INITIAL_SIZE;
		RHI::BufferDesc stagingBufferDesc;
		stagingBufferDesc.size = stagingBufferSize;
		stagingBufferDesc.usage = RHI::BufferUsage::CopySrc;
		RHI::AutomaticAllocationInfo allocInfo;
		allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::Sequential;
		stagingBuffer = device->CreateBuffer(stagingBufferDesc, nullptr, nullptr, &allocInfo, 0, RHI::ResourceType::Automatic).value();

		allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
		auto textureDesc = RHI::TextureDesc{
			.width = 1,
			.height = 1,
			.depthOrArraySize = 1,
			.format = RHI::Format::R8G8B8A8_UNORM,
			.mipLevels = 1,
			.mode = RHI::TextureTilingMode::Optimal,	
		};

		uint8_t whiteData[4] = {255,255,255,255};
		uint8_t blackData[4] = {0,0,0,0};

		whiteTexture.CreateStack(1,1,RHI::Format::R8G8B8A8_UNORM,whiteData PT_DEBUG_REGION(, "RendererBase -> White Texture"));
		blackTexture.CreateStack(1,1,RHI::Format::R8G8B8A8_UNORM,blackData PT_DEBUG_REGION(, "RendererBase -> White Texture"));

		//prep for rendering
		mainCommandList->Begin(commandAllocators[0]);
		if (!options.headless)
		{
			RHI::TextureMemoryBarrier barr;
			barr.AccessFlagsBefore = RHI::ResourceAcessFlags::NONE;
			barr.AccessFlagsAfter = RHI::ResourceAcessFlags::NONE;
			barr.oldLayout = RHI::ResourceLayout::UNDEFINED;
			barr.newLayout = RHI::ResourceLayout::TRANSFER_DST_OPTIMAL;
			barr.subresourceRange.imageAspect = RHI::Aspect::COLOR_BIT;
			barr.subresourceRange.IndexOrFirstMipLevel = 0;
			barr.subresourceRange.NumMipLevels = 1;
			barr.subresourceRange.FirstArraySlice = 0;
			barr.subresourceRange.NumArraySlices = 1;
			barr.texture = backBufferTextures[0];
			barr.previousQueue = barr.nextQueue = RHI::QueueFamily::Ignored;
			mainCommandList->PipelineBarrier(RHI::PipelineStage::TOP_OF_PIPE_BIT, RHI::PipelineStage::TRANSFER_BIT, {},{&barr,1});
		}
		PT_CORE_INFO("Done Initializing RHI");
		if(options.forceSingleQueue) MQ = false;
		return 0;
	}
	void RendererBase::ClearTarget()
	{
		//todo
	}
	void RendererBase::ChangeViewport(int width, int height, int x, int y)
	{
		RHI::Viewport vp;
		vp.width =  (float)width;
		vp.height = (float)height;
		vp.minDepth = 0;
		vp.maxDepth = 1;
		vp.x = (float)x;
		vp.y = (float)y;
		mainCommandList->SetViewports(1, &vp);
	}
	void RendererBase::CreateTarget()
	{
		//todo
	}
	RHI::API RendererBase::GetAPI()
	{
		return instance->GetInstanceAPI();
	}
	void RendererBase::ClearView()
	{
		//todo remove and replace with Begin/End Rendering???
	}

	void RendererBase::Resize(int width, int height)
	{
		//todo implement
	}
	RHI::Instance* RendererBase::GetInstance()
	{
		return instance;
	}
	void RendererBase::PushBufferUpdate(RHI::Weak<RHI::Buffer> buffer, uint32_t offsetFromBufferStart, const void* data, uint32_t size)
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

	void RendererBase::PushTextureUpdate(RHI::Weak<RHI::Texture> texture, uint32_t size ,const void* data,RHI::SubResourceRange* range, RHI::Extent3D imageExtent, RHI::Offset3D imageOffset,RHI::Format format)
	{
		PT_CORE_ASSERT((size % (imageExtent.width * imageExtent.height)) == 0);
		//check required offset
		uint32_t offsetFactor = RHI::Util::GetFormatBPP(format);
		uint32_t currentOffset = staginBufferPortionUsed;
		uint32_t requiredOffset = 0;
		if (currentOffset==0) requiredOffset = 0;
		else
		{
			uint32_t remainder = currentOffset % offsetFactor;
			if (remainder == 0) requiredOffset = currentOffset;
			else requiredOffset = currentOffset + offsetFactor - remainder;
		}
		//check if we have enough space to queue the write
		if ((stagingBufferSize - staginBufferPortionUsed) < size+requiredOffset)
		{
			FlushStagingBuffer();
			requiredOffset = 0;
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
		stagingBufferPointer = ((std::uint8_t*)stagingBufferPointer) + requiredOffset; //offset by the amount of bytes already used
		memcpy(stagingBufferPointer, data, size);
		stagingBuffer->UnMap();
		stagingCommandList->CopyBufferToImage(requiredOffset, *range, imageOffset, imageExtent, stagingBuffer, texture);
		staginBufferPortionUsed = size + requiredOffset;
		outstandingResourceUpdate = true;
	}

	RHI::Ptr<RHI::DescriptorSet> RendererBase::CreateDescriptorSet(RHI::Ptr<RHI::DescriptorSetLayout> layout)
	{
		return device->CreateDescriptorSets(heap, 1, &layout).value()[0];
	}

	void RendererBase::FlushStagingBuffer()
	{
		static uint64_t stagingFenceVal = 0;
		stagingFenceVal++;
		stagingCommandList->End();
		directQueue->ExecuteCommandLists(&stagingCommandList->ID, 1); //todo look into dedicated transfer queue ??
		directQueue->SignalFence(stagingFence, stagingFenceVal);
		stagingFence->Wait(stagingFenceVal); // consider doing this async perhaps;
		stagingCommandAllocator->Reset();
		stagingCommandList->Begin(stagingCommandAllocator);
		staginBufferPortionUsed = 0;
		outstandingResourceUpdate = false;
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



	RHI::Ptr<RHI::Device> RendererBase::Get3dDevice()
	{
		return device;
	}
	RHI::Ptr<RHI::GraphicsCommandList> Pistachio::RendererBase::GetMainCommandList()
	{
		return mainCommandList;
	}
	RTVHandle RendererBase::CreateRenderTargetView(RHI::Weak<RHI::Texture> texture, const RHI::RenderTargetViewDesc& desc)
	{
		if (freeRTVs.size())
		{
			auto handle = freeRTVs[freeRTVs.size() - 1];
			RHI::CPU_HANDLE CPUhandle{};
			CPUhandle.val = rtvHeaps[handle.heapIndex].heap->GetCpuHandle().val +
				device->GetDescriptorHeapIncrementSize(RHI::DescriptorType::RTV) * handle.heapOffset;
			device->CreateRenderTargetView(texture, desc, CPUhandle);
			freeRTVs.pop_back();
			return handle;
		}
		for (uint32_t i = 0; i < rtvHeaps.size();i++)
		{
			auto& heap = rtvHeaps[i];
			if (heap.sizeLeft)
			{
				RHI::CPU_HANDLE CPUhandle{};
				CPUhandle.val = heap.heap->GetCpuHandle().val +
					device->GetDescriptorHeapIncrementSize(RHI::DescriptorType::RTV) * heap.freeOffset;
				device->CreateRenderTargetView(texture, desc, CPUhandle);
				heap.freeOffset++;
				heap.sizeLeft--;
				return RTVHandle{ i, heap.freeOffset - 1 };
			}
		}
		//no space in all heaps
		auto& heap = rtvHeaps.emplace_back();
		RHI::PoolSize pSize;
		pSize.numDescriptors = 10;
		pSize.type = RHI::DescriptorType::RTV;
		RHI::DescriptorHeapDesc hDesc;
		hDesc.maxDescriptorSets = 10;//?
		hDesc.numPoolSizes = 1;
		hDesc.poolSizes = &pSize;
		heap.heap = device->CreateDescriptorHeap(hDesc).value();
		heap.sizeLeft = 10;
		heap.freeOffset = 0;
		return CreateRenderTargetView(texture, desc);

	}
	DSVHandle RendererBase::CreateDepthStencilView(RHI::Weak<RHI::Texture> texture, const RHI::DepthStencilViewDesc& desc)
	{
		if (freeDSVs.size())
		{
			auto handle = freeDSVs[freeDSVs.size() - 1];
			RHI::CPU_HANDLE CPUhandle{};
			CPUhandle.val = dsvHeaps[handle.heapIndex].heap->GetCpuHandle().val +
				device->GetDescriptorHeapIncrementSize(RHI::DescriptorType::DSV) * handle.heapOffset;
			device->CreateDepthStencilView(texture, desc, CPUhandle);
			freeDSVs.pop_back();
			return handle;
		}
		for (uint32_t i = 0; i < dsvHeaps.size(); i++)
		{
			auto& heap = dsvHeaps[i];
			if (heap.sizeLeft)
			{
				RHI::CPU_HANDLE CPUhandle{};
				CPUhandle.val = heap.heap->GetCpuHandle().val +
					device->GetDescriptorHeapIncrementSize(RHI::DescriptorType::DSV) * heap.freeOffset;
				device->CreateDepthStencilView(texture, desc, CPUhandle);
				heap.freeOffset++;
				heap.sizeLeft--;
				return DSVHandle{ i, heap.freeOffset - 1 };
			}
		}
		//no space in all heaps
		auto& heap = dsvHeaps.emplace_back();
		RHI::PoolSize pSize;
		pSize.numDescriptors = 10;
		pSize.type = RHI::DescriptorType::DSV;
		RHI::DescriptorHeapDesc hDesc;
		hDesc.maxDescriptorSets = 10;//?
		hDesc.numPoolSizes = 1;
		hDesc.poolSizes = &pSize;
		heap.heap = device->CreateDescriptorHeap(hDesc).value();
		heap.sizeLeft = 10;
		heap.freeOffset = 0;
		return CreateDepthStencilView(texture, desc);
	}
	SamplerHandle Pistachio::RendererBase::CreateSampler(const RHI::SamplerDesc& viewDesc)
	{
		if (freeSamplers.size())
		{
			auto handle = freeSamplers[freeSamplers.size() - 1];
			RHI::CPU_HANDLE CPUhandle{};
			CPUhandle.val = samplerHeaps[handle.heapIndex].heap->GetCpuHandle().val +
				device->GetDescriptorHeapIncrementSize(RHI::DescriptorType::Sampler) * handle.heapOffset;
			device->CreateSampler(viewDesc, CPUhandle);
			freeSamplers.pop_back();
			return handle;
		}
		for (uint32_t i = 0; i < samplerHeaps.size(); i++)
		{
			auto& heap = samplerHeaps[i];
			if (heap.sizeLeft)
			{
				RHI::CPU_HANDLE CPUhandle{};
				CPUhandle.val = heap.heap->GetCpuHandle().val +
					device->GetDescriptorHeapIncrementSize(RHI::DescriptorType::Sampler) * heap.freeOffset;
				device->CreateSampler(viewDesc, CPUhandle);
				heap.freeOffset++;
				heap.sizeLeft--;
				return SamplerHandle{ i, heap.freeOffset - 1 };
			}
		}
		//no space in all heaps
		auto& heap = samplerHeaps.emplace_back();
		RHI::PoolSize pSize;
		pSize.numDescriptors = 10;
		pSize.type = RHI::DescriptorType::Sampler;
		RHI::DescriptorHeapDesc hDesc;
		hDesc.maxDescriptorSets = 10;//?
		hDesc.numPoolSizes = 1;
		hDesc.poolSizes = &pSize;
		heap.heap = device->CreateDescriptorHeap(hDesc).value();
		heap.sizeLeft = 10;
		heap.freeOffset = 0;
		return CreateSampler(viewDesc);
	}
	void RendererBase::DestroyRenderTargetView(RTVHandle handle)
	{
		freeRTVs.push_back(handle);
	}
	void RendererBase::DestroyDepthStencilView(DSVHandle handle)
	{
		freeDSVs.push_back(handle);
	}
	void Pistachio::RendererBase::DestroySampler(SamplerHandle handle)
	{
		freeSamplers.push_back(handle);
	}
	RHI::CPU_HANDLE RendererBase::GetCPUHandle(RTVHandle handle)
	{
		RHI::CPU_HANDLE retVal;
		retVal.val = rtvHeaps[handle.heapIndex].heap->GetCpuHandle().val +
			device->GetDescriptorHeapIncrementSize(RHI::DescriptorType::RTV) * handle.heapOffset;
		return retVal;
	}
	RHI::CPU_HANDLE RendererBase::GetCPUHandle(DSVHandle handle)
	{
		RHI::CPU_HANDLE retVal;
		retVal.val = dsvHeaps[handle.heapIndex].heap->GetCpuHandle().val +
			device->GetDescriptorHeapIncrementSize(RHI::DescriptorType::DSV) * handle.heapOffset;
		return retVal;
	}
	RHI::CPU_HANDLE Pistachio::RendererBase::GetCPUHandle(SamplerHandle handle)
	{
		RHI::CPU_HANDLE retVal;
		retVal.val = samplerHeaps[handle.heapIndex].heap->GetCpuHandle().val +
			device->GetDescriptorHeapIncrementSize(RHI::DescriptorType::Sampler) * handle.heapOffset;
		return retVal;
	}
	TraceRHI::Context& RendererBase::TraceContext()
	{
		return traceRHICtx;
	}
	RHI::Ptr<RHI::SwapChain>      RendererBase::GetSwapChain() { return swapChain; }
	RHI::Ptr<RHI::DescriptorHeap> RendererBase::GetRTVDescriptorHeap() { return MainRTVheap; }
	RHI::Ptr<RHI::DescriptorHeap> RendererBase::GetDSVDescriptorHeap(){return dsvHeap;}
	RHI::Ptr<RHI::DescriptorHeap> RendererBase::GetMainDescriptorHeap() { return heap; }
	RHI::Ptr<RHI::Texture>        RendererBase::GetBackBufferTexture(uint32_t index) { return backBufferTextures[index]; }
	RHI::Ptr<RHI::Texture>        RendererBase::GetDefaultDepthTexture() { return depthTexture; }
	uint32_t             RendererBase::GetCurrentRTVIndex() { return currentRTVindex; }
	uint32_t             RendererBase::GetCurrentFrameIndex() { return currentFrameIndex;}
	
	
	Texture2D& RendererBase::GetWhiteTexture() { return whiteTexture; }
	Texture2D& RendererBase::GetBlackTexture() { return blackTexture; }
}
