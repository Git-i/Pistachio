#include "ptpch.h"
#include "Core\Device.h"
#include "RenderGraph.h"
namespace Pistachio
{
    const RGTextureInstance RGTextureInstance::Invalid = { UINT32_MAX,UINT32_MAX };
    const RGBufferInstance RGBufferInstance::Invalid = { UINT32_MAX,UINT32_MAX };
    RenderGraph::~RenderGraph()
    {
        for (uint32_t i = 0; i < numGFXCmdLists; i++)
        {
            cmdLists[i].list->Release();
        }
        for (uint32_t i = 0; i < numComputeCmdLists; i++)
        {
            cmdLists[i].list->Release();
        }
        delete[] cmdLists;
    }
    RenderGraph::RenderGraph()
    {
        RendererBase::device->CreateFence(&fence, 0);
    }
    void RenderGraph::SubmitToQueue()
    {
        if(levelTransitionIndices.size())
        RendererBase::directQueue->WaitForFence(fence, passesSortedAndFence[0].second);
        for (uint32_t i = 0; i < levelTransitionIndices.size(); i++)
        {
            uint32_t j = i == 0 ? 0 : levelTransitionIndices[i-1].first;
            cmdLists[i].list->End();
            RendererBase::directQueue->ExecuteCommandLists(&cmdLists[i].list->ID, 1);
            if ((levelTransitionIndices[i].second & PassAction::Signal) != (PassAction)0)
            {
                RendererBase::directQueue->WaitForFence(fence, passesSortedAndFence[j].second + 1);
            }
            if ((levelTransitionIndices[i].second & PassAction::Wait) != (PassAction)0)
            {
                RendererBase::directQueue->SignalFence(fence, passesSortedAndFence[j].second + 2);
            }
        }
        if(computeLevelTransitionIndices.size())
        RendererBase::computeQueue->WaitForFence(fence, computePassesSortedAndFence[0].second);
        for (uint32_t i = 0; i < computeLevelTransitionIndices.size(); i++)
        {
            uint32_t j = i == 0 ? 0 : computeLevelTransitionIndices[i - 1].first;
            cmdLists[i].list->End();
            RendererBase::computeQueue->ExecuteCommandLists(&computeCmdLists[i].list->ID, 1);
            if ((computeLevelTransitionIndices[i].second & PassAction::Signal) != (PassAction)0)
            {
                RendererBase::computeQueue->WaitForFence(fence, computePassesSortedAndFence[j].second + 1);
            }
            if ((computeLevelTransitionIndices[i].second & PassAction::Wait) != (PassAction)0)
            {
                RendererBase::computeQueue->SignalFence(fence, computePassesSortedAndFence[j].second + 2);
            }
        }
        
    }
    void RenderGraph::NewFrame()
    {
        for (uint32_t i = 0; i < numGFXCmdLists; i++)
        {
            cmdLists[i].list->Begin(RendererBase::commandAllocators[RendererBase::currentFrameIndex]);
        }
        for (uint32_t i = 0; i < numComputeCmdLists; i++)
        {
            computeCmdLists[i].list->Begin(RendererBase::commandAllocators[RendererBase::currentFrameIndex]);
        }
    }
    RGTextureHandle RenderGraph::CreateTexture(RenderTexture* texture)
    {
        auto tex = textures.emplace_back(RGTexture(texture->m_ID.Get(), RHI::ResourceLayout::UNDEFINED, RHI::ResourceAcessFlags::NONE, RHI::QueueFamily::Ignored, 0, false, 0,1,texture->m_mipLevels));
        tex.rtvHandle = texture->RTView;
        return RGTextureHandle{ &textures, (uint32_t)(textures.size() - 1) };
    }
    RGTextureHandle RenderGraph::CreateTexture(DepthTexture* texture)
    {
        auto tex = textures.emplace_back(RGTexture(texture->m_ID.Get(), RHI::ResourceLayout::UNDEFINED,RHI::ResourceAcessFlags::NONE, RHI::QueueFamily::Ignored ,0, false, 0,1,texture->m_mipLevels));
        tex.dsvHandle = texture->DSView;
        return RGTextureHandle{ &textures, (uint32_t)(textures.size() - 1)  };
    }
    RGTextureHandle RenderGraph::CreateTexture(RenderCubeMap* texture, uint32_t cubeIndex, uint32_t numSlices)
    {

        auto tex = textures.emplace_back(RGTexture(texture->m_ID.Get(), RHI::ResourceLayout::UNDEFINED, RHI::ResourceAcessFlags::NONE, RHI::QueueFamily::Ignored, 0, true, cubeIndex, numSlices,texture->m_mipLevels));
        tex.rtvHandle = texture->RTViews[cubeIndex];
        return RGTextureHandle{ &textures, (uint32_t)(textures.size() - 1) };
    }
    RGTextureInstance RenderGraph::MakeUniqueInstance(RGTextureHandle texture)
    {
        return RGTextureInstance{ texture.offset, textures[texture.offset].numInstances++ };
    }
    RGBufferInstance RenderGraph::MakeUniqueInstance(RGBufferHandle buffer)
    {
        return RGBufferInstance{ buffer.offset, buffers[buffer.offset].numInstances++ };
    }
    RGBufferHandle RenderGraph::CreateBuffer(RHI::Buffer* buffer, uint32_t offset, uint32_t size, RHI::ResourceAcessFlags access, RHI::QueueFamily family)
    {
        auto buff = buffers.emplace_back(RGBuffer(buffer, offset, size, access, family));
        return RGBufferHandle{ &buffers, (uint32_t)(buffers.size() - 1) };
    }
    RenderPass& RenderGraph::AddPass(RHI::PipelineStage stage, const char* name)
    {
        auto& pass = passes.emplace_back();
        pass.stage = stage;
        pass.name = name;
        dirty = true;
        return pass;
    }
    ComputePass& RenderGraph::AddComputePass(const char* passName)
    {
        auto& pass = computePasses.emplace_back();
        dirty = true;
        return pass;
    }
    RGTextureHandle RenderGraph::CreateTexture(Pistachio::Texture* texture, uint32_t mipSlice, bool isArray , uint32_t arraySlice, uint32_t numSlices,uint32_t numMips, RHI::ResourceLayout layout,RHI::ResourceAcessFlags flags , RHI::QueueFamily family)
    {
        auto tex = textures.emplace_back(RGTexture(texture->m_ID.Get(), layout, flags, family,mipSlice, isArray, arraySlice,numSlices,numMips));
        return RGTextureHandle{ &textures, (uint32_t)(textures.size() - 1) };
    }
    RGTextureHandle RenderGraph::CreateTexture(RHI::Texture* texture, uint32_t mipSlice, bool isArray, uint32_t arraySlice, uint32_t numSlices, uint32_t numMips, RHI::ResourceLayout layout,RHI::ResourceAcessFlags flags, RHI::QueueFamily family)
    {
        auto tex = textures.emplace_back(RGTexture(texture, layout, flags, family, mipSlice,isArray,arraySlice,numSlices,numMips));
        return RGTextureHandle{ &textures, (uint32_t)(textures.size() - 1)};
    }
    void RenderPass::SetShader(Shader* shader)
    {
        pso = shader->GetCurrentPipeline();
        pso->Hold();
    }
    RenderPass::~RenderPass()
    {
    }
    void RenderPass::SetPassArea(const RHI::Area2D& _area)
    {
        area = _area;
    }
    void RenderPass::AddColorInput(AttachmentInfo* info)
    {
        inputs.push_back(*info);
    }
    void RenderPass::AddColorOutput(AttachmentInfo* info)
    {
        outputs.push_back(*info);
    }
    void RenderPass::AddBufferInput(BufferAttachmentInfo* buffer)
    {
        bufferInputs.push_back(*buffer);
    }
    void RenderPass::AddBufferOutput(BufferAttachmentInfo* buffer)
    {
        bufferOutputs.push_back(*buffer);
    }

    ComputePass::~ComputePass()
    {
    }

    void ComputePass::AddColorInput(AttachmentInfo* info)
    {
        inputs.push_back(*info);
    }
    void ComputePass::AddColorOutput(AttachmentInfo* info)
    {
        outputs.push_back(*info);
    }
    void ComputePass::AddBufferInput(BufferAttachmentInfo* buffer)
    {
        bufferInputs.push_back(*buffer);
    }
    void ComputePass::AddBufferOutput(BufferAttachmentInfo* buffer)
    {
        bufferOutputs.push_back(*buffer);
    }
    void ComputePass::SetShader(ComputeShader* shader)
    {
        computePipeline = shader->pipeline;
        computePipeline->Hold();
    }
    void ComputePass::SetShader(RHI::ComputePipeline* pipeline)
    {
        computePipeline = pipeline;
        computePipeline->Hold();
    }
    void RenderPass::SetDepthStencilOutput(AttachmentInfo* info) { dsOutput = *info; }
    RHI::ResourceLayout InputLayout(AttachmentUsage usage)
    {
        switch (usage)
        {
        case Pistachio::AttachmentUsage::Graphics: return RHI::ResourceLayout::SHADER_READ_ONLY_OPTIMAL;
            break;
        case Pistachio::AttachmentUsage::Copy: return RHI::ResourceLayout::TRANSFER_SRC_OPTIMAL;
            break;
        case Pistachio::AttachmentUsage::Compute: return RHI::ResourceLayout::SHADER_READ_ONLY_OPTIMAL;
        default: return RHI::ResourceLayout::UNDEFINED;
            break;
        }
    }
    RHI::ResourceLayout OutputLayout(AttachmentUsage usage)
    {
        switch (usage)
        {
        case Pistachio::AttachmentUsage::Graphics: return RHI::ResourceLayout::COLOR_ATTACHMENT_OPTIMAL;
            break;
        case Pistachio::AttachmentUsage::Copy: return RHI::ResourceLayout::TRANSFER_DST_OPTIMAL;
            break;
        case Pistachio::AttachmentUsage::Compute: return RHI::ResourceLayout::GENERAL;
            break;
        default: return RHI::ResourceLayout::UNDEFINED;
            break;
        }
    }
    RHI::ResourceAcessFlags InputSrcAccess(AttachmentUsage usage)
    {
        switch (usage)
        {
        case Pistachio::AttachmentUsage::Graphics: return RHI::ResourceAcessFlags::NONE;
            break;
        case Pistachio::AttachmentUsage::Copy: return RHI::ResourceAcessFlags::NONE;
            break;
        case Pistachio::AttachmentUsage::Compute: return RHI::ResourceAcessFlags::NONE;
            break;
        default: return RHI::ResourceAcessFlags::NONE;
            break;
        }
    }
    RHI::ResourceAcessFlags InputDstAccess(AttachmentUsage usage)
    {
        switch (usage)
        {
        case Pistachio::AttachmentUsage::Graphics: return RHI::ResourceAcessFlags::SHADER_READ;
            break;
        case Pistachio::AttachmentUsage::Copy: return RHI::ResourceAcessFlags::TRANSFER_READ;
            break;
        case Pistachio::AttachmentUsage::Compute: return RHI::ResourceAcessFlags::SHADER_READ;
            break;
        default: return RHI::ResourceAcessFlags::NONE;
            break;
        }
    }
    RHI::ResourceAcessFlags OutputDstAccess(AttachmentUsage usage)
    {
        switch (usage)
        {
        case Pistachio::AttachmentUsage::Graphics: return RHI::ResourceAcessFlags::COLOR_ATTACHMENT_WRITE;
            break;
        case Pistachio::AttachmentUsage::Copy: return RHI::ResourceAcessFlags::TRANSFER_WRITE;
            break;
        case Pistachio::AttachmentUsage::Compute: return RHI::ResourceAcessFlags::SHADER_WRITE;
        default: return RHI::ResourceAcessFlags::NONE;
            break;
        }
    }
    RHI::ResourceAcessFlags OutputSrcAccess(AttachmentUsage usage)
    {
        switch (usage)
        {
        case Pistachio::AttachmentUsage::Graphics: return RHI::ResourceAcessFlags::NONE;
            break;
        case Pistachio::AttachmentUsage::Copy: return RHI::ResourceAcessFlags::NONE;
            break;
        case Pistachio::AttachmentUsage::Compute: return RHI::ResourceAcessFlags::NONE;
        default: return RHI::ResourceAcessFlags::NONE;
            break;
        }
    }
    void RenderGraph::Execute()
    {
        if (dirty) Compile();
        RHI::PipelineStage stage = RHI::PipelineStage::TOP_OF_PIPE_BIT;

        for (uint32_t i = 0; i < levelTransitionIndices.size(); i++)
        {
            RHI::GraphicsCommandList* currentList = cmdLists[i].list;
            uint32_t j = i == 0 ? 0 : levelTransitionIndices[i - 1].first;
            for (; j < levelTransitionIndices[i].first; j++)
            {
                //transition all inputs to good state
                RenderPass* pass = passesSortedAndFence[j].first;
                RHI::TextureMemoryBarrier* barriers = new RHI::TextureMemoryBarrier[pass->inputs.size() +
                    pass->outputs.size() + 
                    ((pass->dsOutput.texture != RGTextureInstance::Invalid) ? 1 : 0)];//alloca??
                RHI::BufferMemoryBarrier* bufferBarriers = new RHI::BufferMemoryBarrier[pass->bufferInputs.size() + pass->bufferOutputs.size()];
                RHI::RenderingBeginDesc rbDesc{};
                rbDesc.numColorAttachments = (uint32_t)pass->outputs.size();
                rbDesc.renderingArea = pass->area;
                RHI::RenderingAttachmentDesc* attachments = new RHI::RenderingAttachmentDesc[pass->outputs.size() + 
                    ((pass->dsOutput.texture != RGTextureInstance::Invalid) ? 1 : 0)];
                rbDesc.pColorAttachments = attachments;
                uint32_t attachmentCount = 0;
                uint32_t barrierCount = 0;
                uint32_t bufferBarrierCount = 0;
                for (auto& input : pass->inputs)
                {
                    RGTexture& tex = textures[input.texture.texOffset];
                    barriers[barrierCount].newLayout = InputLayout(input.usage);
                    barriers[barrierCount].AccessFlagsAfter = InputDstAccess(input.usage);
                    if (
                        tex.current_layout != barriers[barrierCount].newLayout ||
                        tex.currentAccess != barriers[barrierCount].AccessFlagsAfter ||
                        tex.currentFamily != RHI::QueueFamily::Graphics
                        )
                    {
                        //temporary
                        RHI::SubResourceRange range;
                        range.FirstArraySlice = tex.IsArray ? tex.arraySlice : 0;
                        range.imageAspect = RHI::Aspect::COLOR_BIT;
                        range.IndexOrFirstMipLevel = tex.mipSlice;
                        range.NumArraySlices = tex.IsArray ? tex.sliceCount : 1;
                        range.NumMipLevels = tex.mipSliceCount;
                        //transition
                        barriers[barrierCount].AccessFlagsBefore = InputSrcAccess(input.usage);
                        barriers[barrierCount].oldLayout = RHI::ResourceLayout::UNDEFINED;// tex.current_layout;
                        barriers[barrierCount].texture = tex.texture;
                        barriers[barrierCount].subresourceRange = range;
                        barriers[barrierCount].previousQueue = tex.currentFamily == RHI::QueueFamily::Graphics ? RHI::QueueFamily::Ignored : tex.currentFamily;
                        barriers[barrierCount].nextQueue = tex.currentFamily == RHI::QueueFamily::Graphics ? RHI::QueueFamily::Ignored : RHI::QueueFamily::Graphics;
                        tex.currentFamily = RHI::QueueFamily::Graphics;
                        tex.current_layout = barriers[barrierCount].newLayout;
                        tex.currentAccess = barriers[barrierCount].AccessFlagsAfter;
                        barrierCount++;
                    }
                }
                for (auto& output : pass->outputs)
                {
                    RGTexture& tex = textures[output.texture.texOffset];
                    if (output.usage == AttachmentUsage::Graphics)
                    {
                        attachments[attachmentCount].clearColor = { 0,0,0,0 };
                        attachments[attachmentCount].loadOp = output.loadOp;
                        attachments[attachmentCount].storeOp = RHI::StoreOp::Store;
                        if (tex.rtvHandle.heapIndex == UINT32_MAX)
                        {
                            RHI::RenderTargetViewDesc rtvDesc;
                            rtvDesc.arraySlice = tex.arraySlice;
                            rtvDesc.format = output.format;
                            rtvDesc.TextureArray = tex.IsArray;
                            rtvDesc.textureMipSlice = tex.mipSlice;
                            tex.rtvHandle = RendererBase::CreateRenderTargetView(tex.texture, &rtvDesc);
                        }
                        attachments[attachmentCount].ImageView = RendererBase::GetCPUHandle(tex.rtvHandle);
                        attachmentCount++;
                    }
                    barriers[barrierCount].newLayout = OutputLayout(output.usage);
                    barriers[barrierCount].AccessFlagsAfter = OutputDstAccess(output.usage);
                    barriers[barrierCount].AccessFlagsBefore = OutputSrcAccess(output.usage);
                    if (
                        tex.current_layout != barriers[barrierCount].newLayout ||
                        tex.currentFamily != RHI::QueueFamily::Graphics
                        )
                    {
                        //temporary
                        RHI::SubResourceRange range;
                        range.FirstArraySlice = tex.IsArray ? tex.arraySlice : 0;
                        range.imageAspect = RHI::Aspect::COLOR_BIT;
                        range.IndexOrFirstMipLevel = tex.mipSlice;
                        range.NumArraySlices = tex.IsArray ? tex.sliceCount : 1;
                        range.NumMipLevels = tex.mipSliceCount;
                        //transition
                        barriers[barrierCount].oldLayout = RHI::ResourceLayout::UNDEFINED;// tex.current_layout;
                        barriers[barrierCount].texture = tex.texture;
                        barriers[barrierCount].subresourceRange = range;
                        barriers[barrierCount].previousQueue = tex.currentFamily == RHI::QueueFamily::Graphics ? RHI::QueueFamily::Ignored : tex.currentFamily;
                        barriers[barrierCount].nextQueue =     tex.currentFamily == RHI::QueueFamily::Graphics ? RHI::QueueFamily::Ignored : RHI::QueueFamily::Graphics;
                        tex.currentFamily = RHI::QueueFamily::Graphics;
                        tex.current_layout = barriers[barrierCount].newLayout;
                        tex.currentAccess = barriers[barrierCount].AccessFlagsAfter;
                        barrierCount++;
                    }
                }
                if (pass->dsOutput.texture != RGTextureInstance::Invalid)
                {
                    RGTexture& tex = textures[pass->dsOutput.texture.texOffset];
                    attachments[attachmentCount].clearColor = { 1,1,1,1 };
                    attachments[attachmentCount].loadOp = RHI::LoadOp::Clear;//todo
                    attachments[attachmentCount].storeOp = RHI::StoreOp::Store;
                    if (tex.dsvHandle.heapIndex == UINT32_MAX)
                    {
                        RHI::DepthStencilViewDesc dsvDesc;
                        dsvDesc.arraySlice = tex.arraySlice;
                        dsvDesc.format = pass->dsOutput.format;
                        dsvDesc.TextureArray = tex.IsArray;
                        dsvDesc.textureMipSlice = tex.mipSlice;
                        tex.dsvHandle = RendererBase::CreateDepthStencilView(tex.texture, &dsvDesc);
                    }
                    attachments[attachmentCount].ImageView = RendererBase::GetCPUHandle(tex.dsvHandle);
                    rbDesc.pDepthStencilAttachment = &attachments[attachmentCount];
                    attachmentCount++;
                    if (
                        tex.current_layout != RHI::ResourceLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                        tex.currentAccess != RHI::ResourceAcessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE ||
                        tex.currentFamily != RHI::QueueFamily::Graphics
                        )
                    {
                        //temporary
                        RHI::SubResourceRange range;
                        range.FirstArraySlice = 0;
                        range.imageAspect = RHI::Aspect::DEPTH_BIT;
                        range.IndexOrFirstMipLevel = 0;
                        range.NumArraySlices = 1;
                        range.NumMipLevels = 1;
                        //transition
                        barriers[barrierCount].AccessFlagsAfter = RHI::ResourceAcessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE;//maybe?
                        barriers[barrierCount].AccessFlagsBefore = RHI::ResourceAcessFlags::NONE;
                        barriers[barrierCount].newLayout = RHI::ResourceLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        barriers[barrierCount].oldLayout = tex.current_layout;
                        barriers[barrierCount].texture = tex.texture;
                        barriers[barrierCount].subresourceRange = range;
                        barriers[barrierCount].previousQueue = tex.currentFamily;
                        barriers[barrierCount].nextQueue = RHI::QueueFamily::Graphics;
                        tex.currentFamily = RHI::QueueFamily::Graphics;
                        tex.current_layout = RHI::ResourceLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        tex.currentAccess = RHI::ResourceAcessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE;
                        barrierCount++;
                    }
                }
                for (auto& input : pass->bufferInputs)
                {
                    RGBuffer& buff = buffers[input.buffer.buffOffset];
                    bufferBarriers[bufferBarrierCount].AccessFlagsAfter = InputDstAccess(input.usage);
                    if (
                        buff.currentAccess != bufferBarriers[bufferBarrierCount].AccessFlagsAfter ||
                        buff.currentFamily != RHI::QueueFamily::Compute
                        )
                    {
                        //transition
                        bufferBarriers[bufferBarrierCount].AccessFlagsBefore = InputSrcAccess(input.usage);
                        bufferBarriers[bufferBarrierCount].buffer = buff.buffer;
                        bufferBarriers[bufferBarrierCount].previousQueue = buff.currentFamily;
                        bufferBarriers[bufferBarrierCount].nextQueue = RHI::QueueFamily::Compute;
                        bufferBarriers[bufferBarrierCount].offset = buff.offset;
                        bufferBarriers[bufferBarrierCount].size = buff.size;
                        buff.currentFamily = RHI::QueueFamily::Compute;
                        buff.currentAccess = bufferBarriers[bufferBarrierCount].AccessFlagsAfter;
                        bufferBarrierCount++;
                    }
                }
                for (auto& output : pass->bufferOutputs)
                {
                    RGBuffer& buff = buffers[output.buffer.buffOffset];
                    bufferBarriers[bufferBarrierCount].AccessFlagsAfter = OutputDstAccess(output.usage);
                    if (
                        buff.currentAccess != bufferBarriers[bufferBarrierCount].AccessFlagsAfter ||
                        buff.currentFamily != RHI::QueueFamily::Compute
                        )
                    {
                        //transition
                        bufferBarriers[bufferBarrierCount].AccessFlagsBefore = OutputSrcAccess(output.usage);
                        bufferBarriers[bufferBarrierCount].buffer = buff.buffer;
                        bufferBarriers[bufferBarrierCount].previousQueue = buff.currentFamily;
                        bufferBarriers[bufferBarrierCount].nextQueue = RHI::QueueFamily::Compute;
                        bufferBarriers[bufferBarrierCount].offset = buff.offset;
                        bufferBarriers[bufferBarrierCount].size = buff.size;
                        buff.currentFamily = RHI::QueueFamily::Compute;
                        buff.currentAccess = bufferBarriers[bufferBarrierCount].AccessFlagsAfter;
                        bufferBarrierCount++;
                    }
                }
                //if (pass->stage == RHI::PipelineStage::TRANSFER_BIT) __debugbreak();
                currentList->PipelineBarrier(stage, pass->stage, bufferBarrierCount, bufferBarriers, barrierCount, barriers);
                if (pass->pso.Get()) currentList->SetPipelineState(pass->pso.Get());
                if (attachmentCount) currentList->BeginRendering(&rbDesc);
                pass->pass_fn(currentList);
                stage = pass->stage;
                if (attachmentCount) currentList->EndRendering();
                delete[] barriers;
                delete[] bufferBarriers;
                delete[] attachments;
            }
        }
        for (uint32_t i = 0; i < computeLevelTransitionIndices.size(); i++)
        {
            RHI::GraphicsCommandList* currentList = computeCmdLists[i].list;
            uint32_t j = i == 0 ? 0 : computeLevelTransitionIndices[i - 1].first;
            for (; j < computeLevelTransitionIndices[i].first; j++)
            {
                ComputePass* pass = computePassesSortedAndFence[j].first;
                RHI::TextureMemoryBarrier* barriers = new RHI::TextureMemoryBarrier[pass->inputs.size() + pass->outputs.size()];
                RHI::BufferMemoryBarrier* bufferBarriers = new RHI::BufferMemoryBarrier[pass->bufferInputs.size() + pass->bufferOutputs.size()];
                uint32_t barrierCount = 0;
                uint32_t bufferBarrierCount = 0;
                for (auto& input : pass->inputs)
                {
                    RGTexture& tex = textures[input.texture.texOffset];
                    barriers[barrierCount].newLayout = InputLayout(input.usage);
                    barriers[barrierCount].AccessFlagsAfter = InputSrcAccess(input.usage);
                    if (
                        tex.current_layout != barriers[barrierCount].newLayout ||
                        tex.currentAccess != barriers[barrierCount].AccessFlagsAfter ||
                        tex.currentFamily != RHI::QueueFamily::Compute
                        )
                    {
                        //temporary
                        RHI::SubResourceRange range;
                        range.FirstArraySlice = tex.IsArray ? tex.arraySlice : 0;
                        range.imageAspect = RHI::Aspect::COLOR_BIT;
                        range.IndexOrFirstMipLevel = tex.mipSlice;
                        range.NumArraySlices = 1;
                        range.NumMipLevels = 1;
                        //transition
                        barriers[barrierCount].AccessFlagsBefore = InputSrcAccess(input.usage);
                        barriers[barrierCount].oldLayout = tex.current_layout;
                        barriers[barrierCount].texture = tex.texture;
                        barriers[barrierCount].subresourceRange = range;
                        barriers[barrierCount].previousQueue = tex.currentFamily;
                        barriers[barrierCount].nextQueue = RHI::QueueFamily::Compute;
                        tex.currentFamily = RHI::QueueFamily::Compute;
                        tex.current_layout = barriers[barrierCount].newLayout;
                        tex.currentAccess = barriers[barrierCount].AccessFlagsAfter;
                        barrierCount++;
                    }
                }
                for (auto& output : pass->outputs)
                {
                    RGTexture& tex = textures[output.texture.texOffset];
                    barriers[barrierCount].newLayout = OutputLayout(output.usage);
                    barriers[barrierCount].AccessFlagsAfter = OutputDstAccess(output.usage);
                    if (
                        tex.current_layout != barriers[barrierCount].newLayout ||
                        tex.currentAccess != barriers[barrierCount].AccessFlagsAfter ||
                        tex.currentFamily != RHI::QueueFamily::Compute
                        )
                    {
                        //temporary
                        RHI::SubResourceRange range;
                        range.FirstArraySlice = tex.IsArray ? tex.arraySlice : 0;
                        range.imageAspect = RHI::Aspect::COLOR_BIT;
                        range.IndexOrFirstMipLevel = tex.mipSlice;
                        range.NumArraySlices = 1;
                        range.NumMipLevels = 1;
                        //transition
                        barriers[barrierCount].AccessFlagsBefore = OutputSrcAccess(output.usage);
                        barriers[barrierCount].oldLayout = tex.current_layout;
                        barriers[barrierCount].texture = tex.texture;
                        barriers[barrierCount].subresourceRange = range;
                        barriers[barrierCount].previousQueue = tex.currentFamily;
                        barriers[barrierCount].nextQueue = RHI::QueueFamily::Compute;
                        tex.currentFamily = RHI::QueueFamily::Compute;
                        tex.current_layout = barriers[barrierCount].newLayout;
                        tex.currentAccess = barriers[barrierCount].AccessFlagsAfter;
                        barrierCount++;
                    }
                }
                
                for (auto& input : pass->bufferInputs) 
                {
                    RGBuffer& buff = buffers[input.buffer.buffOffset];
                    bufferBarriers[bufferBarrierCount].AccessFlagsAfter = InputDstAccess(input.usage);
                    if (
                        buff.currentAccess != bufferBarriers[bufferBarrierCount].AccessFlagsAfter ||
                        buff.currentFamily != RHI::QueueFamily::Compute
                        )
                    {
                        //transition
                        bufferBarriers[bufferBarrierCount].AccessFlagsBefore = InputSrcAccess(input.usage);
                        bufferBarriers[bufferBarrierCount].buffer = buff.buffer;
                        bufferBarriers[bufferBarrierCount].previousQueue = buff.currentFamily;
                        bufferBarriers[bufferBarrierCount].nextQueue = RHI::QueueFamily::Compute;
                        bufferBarriers[bufferBarrierCount].offset = buff.offset;
                        bufferBarriers[bufferBarrierCount].size = buff.size;
                        buff.currentFamily = RHI::QueueFamily::Compute;
                        buff.currentAccess = bufferBarriers[bufferBarrierCount].AccessFlagsAfter;
                        bufferBarrierCount++;
                    }
                }
                for (auto& output : pass->bufferOutputs)
                {
                    RGBuffer& buff = buffers[output.buffer.buffOffset];
                    bufferBarriers[bufferBarrierCount].AccessFlagsAfter = OutputDstAccess(output.usage);
                    if (
                        buff.currentAccess != bufferBarriers[bufferBarrierCount].AccessFlagsAfter ||
                        buff.currentFamily != RHI::QueueFamily::Compute
                        )
                    {
                        //transition
                        bufferBarriers[bufferBarrierCount].AccessFlagsBefore = OutputSrcAccess(output.usage);
                        bufferBarriers[bufferBarrierCount].buffer = buff.buffer;
                        bufferBarriers[bufferBarrierCount].previousQueue = buff.currentFamily;
                        bufferBarriers[bufferBarrierCount].nextQueue = RHI::QueueFamily::Compute;
                        bufferBarriers[bufferBarrierCount].offset = buff.offset;
                        bufferBarriers[bufferBarrierCount].size = buff.size;
                        buff.currentFamily = RHI::QueueFamily::Compute;
                        buff.currentAccess = bufferBarriers[bufferBarrierCount].AccessFlagsAfter;
                        bufferBarrierCount++;
                    }
                }
                currentList->PipelineBarrier(RHI::PipelineStage::TOP_OF_PIPE_BIT, RHI::PipelineStage::COMPUTE_SHADER_BIT, bufferBarrierCount, bufferBarriers, barrierCount, barriers);
                currentList->SetComputePipeline(pass->computePipeline.Get());
                pass->pass_fn(currentList);
                delete[] barriers;
                delete[] bufferBarriers;
            }
        }
        
    }

    void RenderGraph::SortPasses()
    {
        std::vector<RenderPass*> passesLeft;
        std::vector<ComputePass*> computePassesLeft;
        for (auto& pass : passes) { passesLeft.push_back(&pass); }
        for (auto& pass : computePasses) { computePassesLeft.push_back(&pass); }
        std::vector<std::tuple<RGTextureInstance, PassType, uint64_t, void*>> readyOutputs;
        std::vector<std::tuple<RGBufferInstance, PassType, uint64_t, void*>> readyBufferOutputs;
        computePassesSortedAndFence.clear();
        passesSortedAndFence.clear();
        while (passesLeft.size() + computePassesLeft.size())
        {
            std::vector<RenderPass*> passesStillLeft;
            std::vector<ComputePass*> computePassesStillLeft;

            //gfx passes
            for (uint32_t i = 0; i < passesLeft.size(); i++)
            {
                RenderPass* pass = passesLeft[i];
                //if all a pass inputs are in the ready Outputs,pass belong to current level
                bool currLevel = true;
                uint64_t fenceVal = 0;
                for (auto& input : pass->inputs)
                {
                    if (auto pos = std::find_if(readyOutputs.begin(), readyOutputs.end(),
                        [input, &fenceVal](const std::tuple<RGTextureInstance, PassType, uint64_t, void*>& output)
                        {
                            bool found = input.texture == std::get<0>(output);
                            bool diffFamily = (std::get<1>(output) != PassType::Graphics);
                            if (found)
                            {
                                fenceVal = std::max(fenceVal, std::get<2>(output) + diffFamily);
                                if (diffFamily)
                                    if (std::get<1>(output) == PassType::Graphics)
                                        ((RenderPass*)std::get<3>(output))->signal = true;
                                    else
                                        ((ComputePass*)std::get<3>(output))->signal = true;
                            }
                            return found;
                        });
                    pos == readyOutputs.end())
                    {
                        passesStillLeft.push_back(pass);
                        currLevel = false;
                        break;
                    }
                }
                for (auto& input : pass->bufferInputs)
                {
                    if (auto pos = std::find_if(readyBufferOutputs.begin(), readyBufferOutputs.end(),
                        [input, &fenceVal](const std::tuple<RGBufferInstance, PassType, uint64_t, void*>& output)
                        {
                            bool found = input.buffer == std::get<0>(output);
                            bool diffFamily = (std::get<1>(output) != PassType::Graphics);
                            if (found)
                            {
                                fenceVal = std::max(fenceVal, std::get<2>(output) + diffFamily);
                                if (diffFamily)
                                    if (std::get<1>(output) == PassType::Graphics)
                                        ((RenderPass*)std::get<3>(output))->signal = true;
                                    else
                                        ((ComputePass*)std::get<3>(output))->signal = true;
                            }
                            return found;
                        });
                    pos == readyBufferOutputs.end())
                    {
                        passesStillLeft.push_back(pass);
                        currLevel = false;
                        break;
                    }
                }
                if (currLevel)
                {
                    passesSortedAndFence.push_back({ pass,fenceVal });
                    for (auto output : pass->outputs)
                    {
                        readyOutputs.push_back({ output.texture ,PassType::Graphics,fenceVal, pass });
                    }
                    for (auto output : pass->bufferOutputs)
                    {
                        readyBufferOutputs.push_back({ output.buffer, PassType::Graphics, fenceVal,pass });
                    }
                    if (pass->dsOutput.texture != RGTextureInstance::Invalid)
                    {
                        readyOutputs.push_back({ pass->dsOutput.texture,PassType::Graphics,fenceVal,pass });
                    };
                }
            }
            //compute passes
            for (uint32_t i = 0; i < computePassesLeft.size(); i++)
            {
                ComputePass* pass = computePassesLeft[i];
                //if all a pass inputs are in the ready Outputs,pass belong to current level
                bool currLevel = true;
                uint64_t fenceVal = 0;
                for (auto& input : pass->inputs)
                {
                    if (auto pos = std::find_if(readyOutputs.begin(), readyOutputs.end(),
                        [input, &fenceVal](const std::tuple<RGTextureInstance, PassType, uint64_t, void*>& output)
                        {
                            bool found = input.texture == std::get<0>(output);
                            bool diffFamily = (std::get<1>(output) != PassType::Compute);
                            if (found)
                            {
                                fenceVal = std::max(fenceVal, std::get<2>(output) + diffFamily);
                                if (diffFamily)
                                    if (std::get<1>(output) == PassType::Graphics)
                                        ((RenderPass*)std::get<3>(output))->signal = true;
                                    else
                                        ((ComputePass*)std::get<3>(output))->signal = true;
                            }
                            return found;
                        });
                    pos == readyOutputs.end())
                    {
                        computePassesStillLeft.push_back(pass);
                        currLevel = false;
                        break;
                    }
                }
                for (auto& input : pass->bufferInputs)
                {
                    if (auto pos = std::find_if(readyBufferOutputs.begin(), readyBufferOutputs.end(),
                        [input, &fenceVal](const std::tuple<RGBufferInstance, PassType, uint64_t, void*>& output)
                        {
                            bool found = input.buffer == std::get<0>(output);
                            bool diffFamily = (std::get<1>(output) != PassType::Compute);
                            if (found)
                            {
                                fenceVal = std::max(fenceVal, std::get<2>(output) + diffFamily);
                                if (diffFamily)
                                    if (std::get<1>(output) == PassType::Graphics)
                                        ((RenderPass*)std::get<3>(output))->signal = true;
                                    else
                                        ((ComputePass*)std::get<3>(output))->signal = true;
                            }
                            return found;
                        });
                    pos == readyBufferOutputs.end())
                    {
                        computePassesStillLeft.push_back(pass);
                        currLevel = false;
                        break;
                    }
                }
                if (currLevel)
                {
                    computePassesSortedAndFence.push_back({ pass,fenceVal });
                    for (auto output : pass->outputs)
                    {
                        readyOutputs.push_back({ output.texture ,PassType::Compute,fenceVal,pass });
                    }
                    for (auto output : pass->bufferOutputs)
                    {
                        readyBufferOutputs.push_back({ output.buffer, PassType::Compute, fenceVal,pass });
                    }
                }
            }
            passesLeft = std::move(passesStillLeft);
            computePassesLeft = std::move(computePassesStillLeft);
            if (passesSortedAndFence.size())
            {
                if (!(levelTransitionIndices.size() && (levelTransitionIndices.end() - 1)->first == passesSortedAndFence.size()))
                    levelTransitionIndices.push_back({ passesSortedAndFence.size(),PassAction::Wait });
            }
            if (computePassesSortedAndFence.size())
            {
                if (!(computeLevelTransitionIndices.size() && (computeLevelTransitionIndices.end() - 1)->first == computePassesSortedAndFence.size()))
                    computeLevelTransitionIndices.push_back({ computePassesSortedAndFence.size(),PassAction::Wait });
            }
        }
        if(levelTransitionIndices.size())(levelTransitionIndices.end() - 1)->second = (PassAction)0;
        if(computeLevelTransitionIndices.size())(computeLevelTransitionIndices.end() - 1)->second = (PassAction)0;
        //Go through every level
        for (uint32_t i = 0; i < levelTransitionIndices.size(); i++)
        {
            uint32_t farthestSignal = 0;
            bool signal = false;
            uint32_t j = i == 0 ? 0 : levelTransitionIndices[i - 1].first;
            for (; j < levelTransitionIndices[i].first; j++)
            {
                if (passesSortedAndFence[j].first->signal)
                {
                    signal = true;
                    farthestSignal = std::max(farthestSignal, j + 1);
                }
            }
            if (signal)
            {
                if (farthestSignal == levelTransitionIndices[i].first)
                {
                    levelTransitionIndices[i].second |= PassAction::Signal;
                }
                else
                {
                    levelTransitionIndices.insert(levelTransitionIndices.begin() + i, { farthestSignal, PassAction::Signal });
                    i++;
                }
            }
        }
        //Go through every Compute level
        for (uint32_t i = 0; i < computeLevelTransitionIndices.size(); i++)
        {
            uint32_t farthestSignal = 0;
            bool signal = false;
            uint32_t j = i == 0 ? 0 : computeLevelTransitionIndices[i - 1].first;
            for (; j < computeLevelTransitionIndices[i].first; j++)
            {
                if (computePassesSortedAndFence[j].first->signal)
                {
                    signal = true;
                    farthestSignal = std::max(farthestSignal, j + 1);
                }
            }
            if (signal)
            {
                if (farthestSignal == computeLevelTransitionIndices[i].first)
                {
                    computeLevelTransitionIndices[i].second |= PassAction::Signal;
                }
                else
                {
                    computeLevelTransitionIndices.insert(computeLevelTransitionIndices.begin() + i, { farthestSignal, PassAction::Signal });
                    i++;
                }
            }
        }


    }
    void RenderGraph::Compile()
    {
        SortPasses();
        if (cmdLists)
        {
            for (uint32_t i = 0; i < numGFXCmdLists; i++)
            {
                cmdLists[i].list->Release();
            }
            delete[] cmdLists;
        }
        if (computeCmdLists)
        {
            for (uint32_t i = 0; i < numComputeCmdLists; i++)
            {
                computeCmdLists[i].list->Release();
            }
            delete[] computeCmdLists;
        }
        numGFXCmdLists = (uint32_t)levelTransitionIndices.size();
        numComputeCmdLists = (uint32_t)computeLevelTransitionIndices.size();
        cmdLists = new RGCommandList[numGFXCmdLists];
        computeCmdLists = new RGCommandList[numComputeCmdLists];
        for (uint32_t i = 0; i < numGFXCmdLists; i++)
        {
            RendererBase::device->CreateCommandList(RHI::CommandListType::Direct,
                RendererBase::commandAllocators[RendererBase::currentFrameIndex], &cmdLists[i].list);
        }
        for (uint32_t i = 0; i < numComputeCmdLists; i++)
        {
            RendererBase::device->CreateCommandList(RHI::CommandListType::Compute,
                RendererBase::computeCommandAllocators[RendererBase::currentFrameIndex], &computeCmdLists[i].list);
        }
        dirty = false;
        NewFrame();
    }

}
