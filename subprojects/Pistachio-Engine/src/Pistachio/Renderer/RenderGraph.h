#pragma once
#include "Texture.h"
#include "Shader.h"
#include "RenderTexture.h"
#include "RendererBase.h"

namespace Pistachio
{
	class RenderPass;
	enum class PassType
	{
		Graphics, Compute
	};
	enum class PassAction : uint32_t
	{
		Signal = 1, Wait = 1 << 1
	};
	ENUM_FLAGS(PassAction);
	class PISTACHIO_API RGBuffer
	{
	public:
		RGBuffer() = default;
		RGBuffer(const RGBuffer&) = default;
		RGBuffer(RGBuffer&&) = default;
		RGBuffer& operator=(const RGBuffer&) = default;
	private:
		friend class RenderGraph;
		RGBuffer(RHI::Buffer* _buffer, uint32_t _offset, uint32_t _size, RHI::QueueFamily family) :
			buffer(_buffer),
			currentFamily(family),
			offset(_offset),
			size  (_size),
			numInstances(1)
		{}
		RHI::Buffer* buffer;
		RHI::QueueFamily currentFamily;
		uint32_t offset;
		uint32_t size;
		uint32_t numInstances;
		RHI::GraphicsCommandList* producer = nullptr;
	};
	class PISTACHIO_API RGTexture
	{
	public:
		RGTexture() = default;
		RGTexture(const RGTexture&) = default;
		RGTexture(RGTexture&&) = default;
		RGTexture& operator=(const RGTexture&) = default;
		void InvalidateRTVHandle()
		{
			//todo invalidate should only destroy if the render graph created it, that is
			//it is not from a pistachio render texture
			if (rtvHandle.heapIndex != UINT32_MAX)
				RendererBase::DestroyRenderTargetView(rtvHandle);
			rtvHandle = { UINT32_MAX, UINT32_MAX };
		}
		//void SetResource(RHI::Texture* texture)
		//{
		//	this->texture = texture;
		//}
	private:
		friend class RenderGraph;
		friend class Renderer;
		friend class Scene;
		RGTexture(RHI::Texture* _texture, RHI::ResourceLayout layout, RHI::QueueFamily family, uint32_t MipSlice, bool isArray, uint32_t Slice, uint32_t numSlices,uint32_t numMips) :
			texture(_texture),
			current_layout(layout),
			mipSlice(MipSlice),
			IsArray(isArray),
			arraySlice(Slice),
			currentFamily(family),
			numInstances(1),
			sliceCount(numSlices),
			mipSliceCount(numMips)
		{}
		
		RHI::Texture* texture;
		RHI::ResourceLayout current_layout;
		uint32_t mipSlice;
		uint32_t mipSliceCount;
		bool IsArray;
		uint32_t arraySlice;
		uint32_t sliceCount;
		RHI::QueueFamily currentFamily;
		RTVHandle rtvHandle = { UINT32_MAX, UINT32_MAX };
		DSVHandle dsvHandle = { UINT32_MAX, UINT32_MAX };//for output resources
		uint32_t numInstances;
		RHI::GraphicsCommandList* producer;

	};
	struct PISTACHIO_API RGTextureInstance
	{
		uint32_t texOffset;//original texture
		uint32_t instID;
		bool operator==(const RGTextureInstance& other) const
		{
			return (texOffset == other.texOffset) && (instID == instID);
		}
		bool operator!=(const RGTextureInstance& other) const
		{
			return !((*this) == other);
		}
		static const RGTextureInstance Invalid;
	};
	
	struct PISTACHIO_API RGBufferInstance
	{
		uint32_t buffOffset;
		uint32_t instID;
		bool operator==(const RGBufferInstance& other) const
		{
			return (buffOffset == other.buffOffset) && (instID == instID);
		}
		bool operator!=(const RGBufferInstance& other) const
		{
			return !((*this) == other);
		}
		static const RGBufferInstance Invalid;
	};
	//instance 0 is used as default instance
	struct RGTextureHandle 
	{
		std::vector<RGTexture>* originVector;
		uint32_t offset;
		operator RGTextureInstance() const
		{
			return RGTextureInstance{ offset, 0 };
		}
	};
	struct RGBufferHandle
	{
		std::vector<RGBuffer>* originVector;
		uint32_t offset;
		operator RGBufferInstance() const
		{
			return RGBufferInstance{ offset, 0 };
		}
	};
	enum class AttachmentUsage
	{
		Graphics,//I: Shader Read, O: Color attachment
		Copy, //I: Copy Src, O: Copy Dst
		Compute, //I: Shader Read, O: Shader Write
		Unspec //Same as graphics but doesnt make attachments
	};
	struct AttachmentInfo
	{
		RHI::Format format;
		RGTextureInstance texture;
		RHI::LoadOp loadOp = RHI::LoadOp::Clear;
		AttachmentUsage usage = AttachmentUsage::Graphics;
	};
	struct BufferAttachmentInfo
	{
		RGBufferInstance buffer;
		AttachmentUsage usage;
	};

	class PISTACHIO_API RenderPass
	{
	public:
		~RenderPass();
		void SetPassArea(const RHI::Area2D& area);
		void AddColorInput(AttachmentInfo* info);
		void AddColorOutput(AttachmentInfo* info);
		void AddBufferInput(BufferAttachmentInfo* buffer);
		void AddBufferOutput(BufferAttachmentInfo* buffer);
		void SetShader(Shader* shader);//Make sure the shader is already preconfigured to desired state
		void SetDepthStencilOutput(AttachmentInfo* info);
		std::function<void(RHI::GraphicsCommandList* list)> pass_fn;
	private:
		friend class RenderGraph;
		RHI::PipelineStage stage;
		RHI::Area2D area;
		const char* name;//temp
		RHIPtr<RHI::PipelineStateObject> pso = nullptr;
		std::vector<AttachmentInfo> inputs;
		std::vector<AttachmentInfo> outputs;
		std::vector<BufferAttachmentInfo> bufferInputs;
		std::vector<BufferAttachmentInfo> bufferOutputs;
		AttachmentInfo dsOutput = { RHI::Format::UNKNOWN,RGTextureInstance::Invalid };
		bool signal = false;
	};
	//unlike the render-pass compute pipeline must be set
	class PISTACHIO_API ComputePass
	{
	public:
		~ComputePass();
		void AddColorInput(AttachmentInfo* info);
		void AddColorOutput(AttachmentInfo* info);
		void AddBufferInput(BufferAttachmentInfo* buffer);
		void AddBufferOutput(BufferAttachmentInfo* buffer);
		void SetShader(ComputeShader* shader);
		void SetShader(RHI::ComputePipeline* pipeline);
		std::function<void(RHI::GraphicsCommandList* list)> pass_fn;
	private:
		friend class RenderGraph;
		RHIPtr<RHI::ComputePipeline> computePipeline = nullptr;
		std::vector<AttachmentInfo> inputs;
		std::vector<AttachmentInfo> outputs;
		std::vector<BufferAttachmentInfo> bufferInputs;
		std::vector<BufferAttachmentInfo> bufferOutputs;
		bool signal = false;
		const char* passName;
	};
	struct RGCommandList
	{
		RHI::GraphicsCommandList* list;
		std::mutex listMutex; //so multiple threads cant record to the list at once and change the pipline state
	};
	//we can possibly have 3 cmd lists and for every independent pass use those three
	/*
	* Every Pass in a graph can have multiple inputs and outputs
	* An output cannot be used as an output for another pass, because then pass dependencies would be screwed up
	* Pass inputs can be shared tho, i think
	*/
	class PISTACHIO_API RenderGraph
	{
	public:
		~RenderGraph();
		RenderGraph();
		void Compile();
		void SubmitToQueue();
		void NewFrame();
		RenderPass& AddPass(RHI::PipelineStage stage, const char* passName);
		ComputePass& AddComputePass(const char* passName);
		void RemovePass(const char* passName);
		void GetPass(const char* passName);
		RGTextureHandle CreateTexture(Pistachio::Texture* texture, uint32_t mipSlice = 0, bool isArray = false, uint32_t arraySlice = 0,uint32_t numSlices = 1, uint32_t numMips = 1, RHI::ResourceLayout = RHI::ResourceLayout::UNDEFINED, RHI::QueueFamily family = RHI::QueueFamily::Graphics);
		RGTextureHandle CreateTexture(RHI::Texture* texture , uint32_t mipSlice = 0, bool isArray = false, uint32_t arraySlice = 0, uint32_t numSlices = 1,uint32_t numMips = 1, RHI::ResourceLayout = RHI::ResourceLayout::UNDEFINED, RHI::QueueFamily family= RHI::QueueFamily::Graphics);
		RGTextureHandle CreateTexture(RenderTexture* texture);
		RGTextureHandle CreateTexture(DepthTexture* texture);
		RGTextureHandle CreateTexture(RenderCubeMap* texture, uint32_t cubeIndex, uint32_t numSlices = 1);
		RGTextureInstance MakeUniqueInstance(RGTextureHandle texture);
		RGBufferInstance MakeUniqueInstance(RGBufferHandle buffer);
		RGBufferHandle CreateBuffer(RHI::Buffer* buffer, uint32_t offset, uint32_t size, RHI::QueueFamily family = RHI::QueueFamily::Graphics);
		void Execute();
	private:
		inline void ExecuteGFXLevel(uint32_t levelInd, RHI::PipelineStage& stage, RHI::GraphicsCommandList* prevList,RHI::QueueFamily srcQueue);
		inline void ExecuteCMPLevel(uint32_t levelInd, RHI::PipelineStage& stage, RHI::GraphicsCommandList* prevList,RHI::QueueFamily srcQueue);
		void SortPasses();
	private:
		friend class Renderer;
		friend class Scene;
		bool dirty = true;
		RHI::DebugBuffer* dbgBufferGFX;
		RHI::DebugBuffer* dbgBufferCMP;
		RHI::Fence* fence;
		std::vector<RGTexture> textures;
		std::vector<RGBuffer> buffers;
		std::vector<RenderPass> passes;
		std::vector<ComputePass> computePasses;
		std::vector<std::pair<RenderPass*, uint32_t>> passesSortedAndFence;
		std::vector<std::pair<ComputePass*, uint32_t>> computePassesSortedAndFence;
		std::vector<std::pair<uint32_t, PassAction>> levelTransitionIndices;
		std::vector<std::pair<uint32_t, PassAction>> computeLevelTransitionIndices;
		uint64_t maxFence = 0;
		RGCommandList* cmdLists = 0;
		uint32_t numGFXCmdLists;
		RGCommandList* computeCmdLists = 0;
		uint32_t numComputeCmdLists;
	};
}
