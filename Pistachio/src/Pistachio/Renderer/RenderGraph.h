#pragma once
#include "Texture.h"
#include "RenderTexture.h"
#include "RendererBase.h"
namespace Pistachio
{
	class RenderPass;
	class PISTACHIO_API RGTexture
	{
	public:
		void InvalidateRTVHandle()
		{
			//todo invalidate should only destroy if the render graph created it, that is
			//it is not from a pistachio render texture
			if(rtvHandle.heapIndex != UINT32_MAX)
				RendererBase::DestroyRenderTargetView(rtvHandle);
			rtvHandle = { UINT32_MAX, UINT32_MAX };
		}
		void SetResource(RHI::Texture* texture)
		{
			this->texture = texture;
		}
	private:
		friend class RenderGraph;
		RGTexture(RHI::Texture* _texture, RHI::ResourceLayout layout, uint32_t MipSlice,bool isArray,uint32_t Slice) : 
			texture(_texture), 
			current_layout(layout),
			mipSlice(MipSlice),
			IsArray(isArray),
			arraySlice(Slice){}
		RGTexture() = default; 
		RGTexture(const RGTexture&) = default; 
		RGTexture(RGTexture&&) = default;
		RHI::Texture* texture;
		RHI::ResourceLayout current_layout;
		uint32_t mipSlice;
		bool IsArray;
		uint32_t arraySlice;
		//every resource can only have i input user and on output user
		RTVHandle rtvHandle = { UINT32_MAX, UINT32_MAX };
		DSVHandle dsvHandle = { UINT32_MAX, UINT32_MAX };//for output resources
		RenderPass* inputUsers;
		RenderPass* outputUsers;
	};
	struct AttachmentInfo
	{
		RHI::Format format;
		RGTexture* texture;
	};

	class PISTACHIO_API RenderPass
	{
	public:
		void SetPassArea(const RHI::Area2D& area);
		void AddColorInput(AttachmentInfo* info);
		void AddColorOutput(AttachmentInfo* info);
		void SetDepthStencilOutput(AttachmentInfo* info);
		std::function<void(RHI::GraphicsCommandList* list)> pass_fn;
	private:
		friend class RenderGraph;
		RHI::PipelineStage stage;
		RHI::Area2D area;
		std::vector<AttachmentInfo> inputs;
		std::vector<AttachmentInfo> outputs;
		AttachmentInfo dsOutput = { (RHI::Format)0,nullptr };
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
		RenderGraph(uint32_t cmdListCount);
		void SubmitToQueue();
		void NewFrame();
		RenderPass& AddPass(RHI::PipelineStage stage, const char* passName);
		void RemovePass(const char* passName);
		void GetPass(const char* passName);
		RGTexture* CreateTexture(Pistachio::Texture* texture, uint32_t mipSlice = 0, bool isArray = false, uint32_t arraySlice = 0,RHI::ResourceLayout = RHI::ResourceLayout::UNDEFINED);
		RGTexture* CreateTexture(RHI::Texture* texture , uint32_t mipSlice = 0, bool isArray = false, uint32_t arraySlice = 0,RHI::ResourceLayout = RHI::ResourceLayout::UNDEFINED);
		RGTexture* CreateTexture(RenderTexture* texture);
		RGTexture* CreateTexture(RenderCubeMap* texture, uint32_t cubeIndex);
		void Execute();
	private:
		bool ValidateGraph();
		void SortPasses();
	private:
		std::vector<RGTexture*> textures;
		std::vector<RenderPass> passes;
		std::vector<RenderPass*> passesSorted;
		std::vector<uint32_t> levelTransitionIndices;
		RGCommandList* cmdLists;
		uint32_t numCmdLists;
	};
}
