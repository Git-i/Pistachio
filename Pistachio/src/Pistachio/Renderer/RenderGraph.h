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
		Signal = 1, Wait = 0
	};
	ENUM_FLAGS(PassAction);
	class PISTACHIO_API RGBuffer
	{
	public:
	private:
		friend class RenderGraph;
		RGBuffer() {}
		RHI::Buffer* buffer;
		RHI::ResourceAcessFlags currentAccess;
		RHI::QueueFamily currentFamily;
		uint32_t offset;
		uint32_t size;
	};
	class PISTACHIO_API RGTexture
	{
	public:
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
		RGTexture(RHI::Texture* _texture, RHI::ResourceLayout layout, RHI::ResourceAcessFlags access, RHI::QueueFamily family, uint32_t MipSlice, bool isArray, uint32_t Slice) :
			texture(_texture),
			current_layout(layout),
			mipSlice(MipSlice),
			IsArray(isArray),
			arraySlice(Slice),
			currentAccess(access),
			currentFamily(family){}
		RGTexture() = default;
		RGTexture(const RGTexture&) = default;
		RGTexture(RGTexture&&) = default;
		RHI::Texture* texture;
		RHI::ResourceLayout current_layout;
		uint32_t mipSlice;
		bool IsArray;
		uint32_t arraySlice;
		RHI::ResourceAcessFlags currentAccess;
		RHI::QueueFamily currentFamily;
		//every resource can only have i input user and on output user
		RTVHandle rtvHandle = { UINT32_MAX, UINT32_MAX };
		DSVHandle dsvHandle = { UINT32_MAX, UINT32_MAX };//for output resources

	};;
	enum class AttachmentUsage
	{
		Graphics,//I: Shader Read, O: Color attachment
		Copy, //I: Copy Src, O: Copy Dst
		Compute //I: Shader Read, O: Shader Write
	};
	struct AttachmentInfo
	{
		RHI::Format format;
		RGTexture* texture;
		RHI::LoadOp loadOp = RHI::LoadOp::Clear;
		AttachmentUsage usage = AttachmentUsage::Graphics;
	};
	struct BufferAttachmentInfo
	{
		RGBuffer* buffer;
		AttachmentUsage usage;
	};

	class PISTACHIO_API RenderPass
	{
	public:
		void SetPassArea(const RHI::Area2D& area);
		void AddColorInput(AttachmentInfo* info);
		void AddColorOutput(AttachmentInfo* info);
		void AddBufferInput(RGBuffer* buffer);
		void AddBufferOutput(RGBuffer* buffer);
		void SetShader(Shader* shader);//Make sure the shader is already preconfigured to desired state
		void SetDepthStencilOutput(AttachmentInfo* info);
		std::function<void(RHI::GraphicsCommandList* list)> pass_fn;
	private:
		friend class RenderGraph;
		RHI::PipelineStage stage;
		RHI::Area2D area;
		RHI::PipelineStateObject* pso = nullptr;
		std::vector<AttachmentInfo> inputs;
		std::vector<AttachmentInfo> outputs;
		std::vector<BufferAttachmentInfo> bufferInputs;
		std::vector<BufferAttachmentInfo> bufferOutputs;
		AttachmentInfo dsOutput = { (RHI::Format)0,nullptr };
		bool signal = false;
	};
	//unlike the render-pass compute pipeline must be set
	class PISTACHIO_API ComputePass
	{
	public:
		std::function<void(RHI::GraphicsCommandList* list)> pass_fn;
	private:
		friend class RenderGraph;
		RHI::ComputePipeline* computePipeline = nullptr;
		std::vector<AttachmentInfo> inputs;
		std::vector<AttachmentInfo> outputs;
		std::vector<BufferAttachmentInfo> bufferInputs;
		std::vector<BufferAttachmentInfo> bufferOutputs;
		bool signal = false;
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
		RenderGraph(uint32_t cmdListCount);
		void Compile();
		void SubmitToQueue();
		void NewFrame();
		RenderPass& AddPass(RHI::PipelineStage stage, const char* passName);
		void RemovePass(const char* passName);
		void GetPass(const char* passName);
		RGTexture* CreateTexture(Pistachio::Texture* texture, uint32_t mipSlice = 0, bool isArray = false, uint32_t arraySlice = 0,RHI::ResourceLayout = RHI::ResourceLayout::UNDEFINED);
		RGTexture* CreateTexture(RHI::Texture* texture , uint32_t mipSlice = 0, bool isArray = false, uint32_t arraySlice = 0,RHI::ResourceLayout = RHI::ResourceLayout::UNDEFINED);
		RGTexture* CreateTexture(RenderTexture* texture);
		RGTexture* CreateTexture(DepthTexture* texture);
		RGTexture* CreateTexture(RenderCubeMap* texture, uint32_t cubeIndex);
		void Execute();
	private:
		void SortPasses();
	private:
		bool dirty = true;
		RHI::Fence* fence;
		std::vector<RGTexture*> textures;
		std::vector<RGBuffer*> buffers;
		std::vector<RenderPass> passes;
		std::vector<ComputePass> computePasses;
		std::vector<std::pair<RenderPass*, uint32_t>> passesSortedAndFence;
		std::vector<std::pair<ComputePass*, uint32_t>> computePassesSortedAndFence;
		std::vector<std::pair<uint32_t, PassAction>> levelTransitionIndices;
		std::vector<std::pair<uint32_t, PassAction>> computeLevelTransitionIndices;
		RGCommandList* cmdLists = 0;
		uint32_t numGFXCmdLists;
		RGCommandList* computeCmdLists = 0;
		uint32_t numComputeCmdLists;
	};
}
