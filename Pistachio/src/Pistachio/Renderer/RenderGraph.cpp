#include "ptpch.h"
#include "Core\Device.h"
#include "RenderGraph.h"
namespace Pistachio
{
    RenderGraph::~RenderGraph()
    {
        for (auto tex : textures)
        {
            delete tex;
        }
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
    RenderGraph::RenderGraph(uint32_t numCmdLists)
    {
        
    }
    void RenderGraph::SubmitToQueue()
    {
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
    RGTexture* RenderGraph::CreateTexture(RenderTexture* texture)
    {
        auto tex = textures.emplace_back(new RGTexture(texture->m_ID.Get(), RHI::ResourceLayout::UNDEFINED, RHI::ResourceAcessFlags::NONE, RHI::QueueFamily::Ignored, 0, false, 0));
        tex->rtvHandle = texture->RTView;
        return tex;
    }
    RGTexture* RenderGraph::CreateTexture(DepthTexture* texture)
    {
        auto tex = textures.emplace_back(new RGTexture(texture->m_ID.Get(), RHI::ResourceLayout::UNDEFINED,RHI::ResourceAcessFlags::NONE, RHI::QueueFamily::Ignored ,0, false, 0));
        tex->dsvHandle = texture->DSView;
        return tex;
    }
    RGTexture* RenderGraph::CreateTexture(RenderCubeMap* texture, uint32_t cubeIndex)
    {
        auto tex = textures.emplace_back(new RGTexture(texture->m_ID.Get(), RHI::ResourceLayout::UNDEFINED, RHI::ResourceAcessFlags::NONE, RHI::QueueFamily::Ignored, 0, true, cubeIndex));
        tex->rtvHandle = texture->RTViews[cubeIndex];
        return tex;
    }
    RGBuffer* RenderGraph::CreateBuffer(RHI::Buffer* buffer, uint32_t offset, uint32_t size, RHI::ResourceAcessFlags access, RHI::QueueFamily family)
    {
        auto buff = buffers.emplace_back(new RGBuffer(buffer, offset, size, access, family));
        return buff;
    }
    RenderPass& RenderGraph::AddPass(RHI::PipelineStage stage, const char* name)
    {
        auto& pass = passes.emplace_back();
        pass.stage = stage;
        dirty = true;
        return pass;
    }
    ComputePass& RenderGraph::AddComputePass(const char* passName)
    {
        auto& pass = computePasses.emplace_back();
        dirty = true;
        return pass;
    }
    RGTexture* RenderGraph::CreateTexture(Pistachio::Texture* texture, uint32_t mipSlice, bool isArray , uint32_t arraySlice, RHI::ResourceLayout layout,RHI::ResourceAcessFlags flags , RHI::QueueFamily family)
    {
        auto tex = textures.emplace_back(new RGTexture(texture->m_ID.Get(), layout, flags, family,mipSlice, isArray, arraySlice));
        return tex;
    }
    RGTexture* RenderGraph::CreateTexture(RHI::Texture* texture, uint32_t mipSlice, bool isArray, uint32_t arraySlice, RHI::ResourceLayout layout,RHI::ResourceAcessFlags flags, RHI::QueueFamily family)
    {
        auto tex = textures.emplace_back(new RGTexture(texture, layout, flags, family, mipSlice,isArray,arraySlice));
        return tex;
    }
    void RenderPass::SetShader(Shader* shader)
    {
        if (pso) pso->Release();
        pso = shader->GetCurrentPipeline();
        pso->Hold();
    }
    RenderPass::~RenderPass()
    {
        if (pso) pso->Release();
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
        if (computePipeline) computePipeline->Release();
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
        if (computePipeline) computePipeline->Release();
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
        default:
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
        default:
            break;
        }
    }
    RHI::ResourceAcessFlags InputAccess(AttachmentUsage usage)
    {
        switch (usage)
        {
        case Pistachio::AttachmentUsage::Graphics: return RHI::ResourceAcessFlags::SHADER_READ;
            break;
        case Pistachio::AttachmentUsage::Copy: return RHI::ResourceAcessFlags::TRANSFER_READ;
            break;
        case Pistachio::AttachmentUsage::Compute: return RHI::ResourceAcessFlags::SHADER_READ;
            break;
        default:
            break;
        }
    }
    RHI::ResourceAcessFlags OutputAccess(AttachmentUsage usage)
    {
        switch (usage)
        {
        case Pistachio::AttachmentUsage::Graphics: return RHI::ResourceAcessFlags::COLOR_ATTACHMENT_WRITE;
            break;
        case Pistachio::AttachmentUsage::Copy: return RHI::ResourceAcessFlags::TRANSFER_WRITE;
            break;
        case Pistachio::AttachmentUsage::Compute: return RHI::ResourceAcessFlags::SHADER_WRITE;
        default:
            break;
        }
    }
    void RenderGraph::Execute()
    {
        if (dirty) Compile();
        for (uint32_t i = 0; i < levelTransitionIndices.size(); i++)
        {
            RHI::GraphicsCommandList* currentList = cmdLists[i].list;
            uint32_t j = i == 0 ? 0 : levelTransitionIndices[i - 1].first;
            for (; j < levelTransitionIndices[i].first; j++)
            {
                //transition all inputs to good state
                RenderPass* pass = passesSortedAndFence[j].first;
                RHI::TextureMemoryBarrier* barriers = new RHI::TextureMemoryBarrier[pass->inputs.size() + pass->outputs.size() + (pass->dsOutput.texture ? 1 : 0)];//alloca??
                RHI::BufferMemoryBarrier* bufferBarriers = new RHI::BufferMemoryBarrier[pass->bufferInputs.size() + pass->bufferOutputs.size()];
                RHI::RenderingBeginDesc rbDesc{};
                rbDesc.numColorAttachments = pass->outputs.size();
                rbDesc.renderingArea = pass->area;
                RHI::RenderingAttachmentDesc* attachments = new RHI::RenderingAttachmentDesc[pass->outputs.size() + (pass->dsOutput.texture ? 1 : 0)];
                rbDesc.pColorAttachments = attachments;
                uint32_t attachmentCount = 0;
                uint32_t barrierCount = 0;
                uint32_t bufferBarrierCount = 0;
                for (auto& input : pass->inputs)
                {
                    barriers[barrierCount].newLayout = InputLayout(input.usage);
                    barriers[barrierCount].AccessFlagsAfter = InputAccess(input.usage);
                    if (
                        input.texture->current_layout != barriers[barrierCount].newLayout ||
                        input.texture->currentAccess != barriers[barrierCount].AccessFlagsAfter ||
                        input.texture->currentFamily != RHI::QueueFamily::Graphics
                        )
                    {
                        //temporary
                        RHI::SubResourceRange range;
                        range.FirstArraySlice = input.texture->IsArray ? input.texture->arraySlice : 0;
                        range.imageAspect = RHI::Aspect::COLOR_BIT;
                        range.IndexOrFirstMipLevel = input.texture->mipSlice;
                        range.NumArraySlices = 1;
                        range.NumMipLevels = 1;
                        //transition
                        barriers[barrierCount].AccessFlagsBefore = input.texture->currentAccess;
                        barriers[barrierCount].oldLayout = input.texture->current_layout;
                        barriers[barrierCount].texture = input.texture->texture;
                        barriers[barrierCount].subresourceRange = range;
                        barriers[barrierCount].previousQueue = input.texture->currentFamily;
                        barriers[barrierCount].nextQueue = RHI::QueueFamily::Graphics;
                        input.texture->currentFamily = RHI::QueueFamily::Graphics;
                        input.texture->current_layout = barriers[barrierCount].newLayout;
                        input.texture->currentAccess = barriers[barrierCount].AccessFlagsAfter;
                        barrierCount++;
                    }
                }
                for (auto& output : pass->outputs)
                {
                    if (output.usage == AttachmentUsage::Graphics)
                    {
                        attachments[attachmentCount].clearColor = { 0,0,0,0 };
                        attachments[attachmentCount].loadOp = output.loadOp;
                        attachments[attachmentCount].storeOp = RHI::StoreOp::Store;
                        if (output.texture->rtvHandle.heapIndex == UINT32_MAX)
                        {
                            RHI::RenderTargetViewDesc rtvDesc;
                            rtvDesc.arraySlice = output.texture->arraySlice;
                            rtvDesc.format = output.format;
                            rtvDesc.TextureArray = output.texture->IsArray;
                            rtvDesc.textureMipSlice = output.texture->mipSlice;
                            output.texture->rtvHandle = RendererBase::CreateRenderTargetView(output.texture->texture, &rtvDesc);
                        }
                        attachments[attachmentCount].ImageView = RendererBase::GetCPUHandle(output.texture->rtvHandle);
                        attachmentCount++;
                    }
                    barriers[barrierCount].newLayout = OutputLayout(output.usage);
                    barriers[barrierCount].AccessFlagsAfter = OutputAccess(output.usage);
                    if (
                        output.texture->current_layout != barriers[barrierCount].newLayout ||
                        output.texture->currentAccess != barriers[barrierCount].AccessFlagsAfter ||
                        output.texture->currentFamily != RHI::QueueFamily::Graphics
                        )
                    {
                        //temporary
                        RHI::SubResourceRange range;
                        range.FirstArraySlice = output.texture->IsArray ? output.texture->arraySlice : 0;
                        range.imageAspect = RHI::Aspect::COLOR_BIT;
                        range.IndexOrFirstMipLevel = output.texture->mipSlice;
                        range.NumArraySlices = 1;
                        range.NumMipLevels = 1;
                        //transition
                        barriers[barrierCount].AccessFlagsBefore = output.texture->currentAccess;
                        barriers[barrierCount].oldLayout = output.texture->current_layout;
                        barriers[barrierCount].texture = output.texture->texture;
                        barriers[barrierCount].subresourceRange = range;
                        barriers[barrierCount].previousQueue = output.texture->currentFamily;
                        barriers[barrierCount].nextQueue = RHI::QueueFamily::Graphics;
                        output.texture->currentFamily = RHI::QueueFamily::Graphics;
                        output.texture->current_layout = barriers[barrierCount].newLayout;
                        output.texture->currentAccess = barriers[barrierCount].AccessFlagsAfter;
                        barrierCount++;
                    }
                }
                if (pass->dsOutput.texture)
                {
                    attachments[attachmentCount].clearColor = { 1,1,1,1 };
                    attachments[attachmentCount].loadOp = RHI::LoadOp::Clear;//todo
                    attachments[attachmentCount].storeOp = RHI::StoreOp::Store;
                    if (pass->dsOutput.texture->dsvHandle.heapIndex == UINT32_MAX)
                    {
                        RHI::DepthStencilViewDesc dsvDesc;
                        dsvDesc.arraySlice = pass->dsOutput.texture->arraySlice;
                        dsvDesc.format = pass->dsOutput.format;
                        dsvDesc.TextureArray = pass->dsOutput.texture->IsArray;
                        dsvDesc.textureMipSlice = pass->dsOutput.texture->mipSlice;
                        pass->dsOutput.texture->dsvHandle = RendererBase::CreateDepthStencilView(pass->dsOutput.texture->texture, &dsvDesc);
                    }
                    attachments[attachmentCount].ImageView = RendererBase::GetCPUHandle(pass->dsOutput.texture->dsvHandle);
                    rbDesc.pDepthStencilAttachment = &attachments[attachmentCount];
                    attachmentCount++;
                    if (
                        pass->dsOutput.texture->current_layout != RHI::ResourceLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                        pass->dsOutput.texture->currentAccess != RHI::ResourceAcessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE ||
                        pass->dsOutput.texture->currentFamily != RHI::QueueFamily::Graphics
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
                        barriers[barrierCount].AccessFlagsBefore = pass->dsOutput.texture->currentAccess;
                        barriers[barrierCount].newLayout = RHI::ResourceLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        barriers[barrierCount].oldLayout = pass->dsOutput.texture->current_layout;
                        barriers[barrierCount].texture = pass->dsOutput.texture->texture;
                        barriers[barrierCount].subresourceRange = range;
                        barriers[barrierCount].previousQueue = pass->dsOutput.texture->currentFamily;
                        barriers[barrierCount].nextQueue = RHI::QueueFamily::Graphics;
                        pass->dsOutput.texture->currentFamily = RHI::QueueFamily::Graphics;
                        pass->dsOutput.texture->current_layout = RHI::ResourceLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        pass->dsOutput.texture->currentAccess = RHI::ResourceAcessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE;
                        barrierCount++;
                    }
                }
                for (auto& input : pass->bufferInputs)
                {
                    bufferBarriers[bufferBarrierCount].AccessFlagsAfter = InputAccess(input.usage);
                    if (
                        input.buffer->currentAccess != bufferBarriers[bufferBarrierCount].AccessFlagsAfter ||
                        input.buffer->currentFamily != RHI::QueueFamily::Compute
                        )
                    {
                        //transition
                        bufferBarriers[bufferBarrierCount].AccessFlagsBefore = input.buffer->currentAccess;
                        bufferBarriers[bufferBarrierCount].buffer = input.buffer->buffer;
                        bufferBarriers[bufferBarrierCount].previousQueue = input.buffer->currentFamily;
                        bufferBarriers[bufferBarrierCount].nextQueue = RHI::QueueFamily::Compute;
                        bufferBarriers[bufferBarrierCount].offset = input.buffer->offset;
                        bufferBarriers[bufferBarrierCount].size = input.buffer->size;
                        input.buffer->currentFamily = RHI::QueueFamily::Compute;
                        input.buffer->currentAccess = bufferBarriers[bufferBarrierCount].AccessFlagsAfter;
                        bufferBarrierCount++;
                    }
                }
                for (auto& output : pass->bufferOutputs)
                {
                    bufferBarriers[bufferBarrierCount].AccessFlagsAfter = OutputAccess(output.usage);
                    if (
                        output.buffer->currentAccess != bufferBarriers[bufferBarrierCount].AccessFlagsAfter ||
                        output.buffer->currentFamily != RHI::QueueFamily::Compute
                        )
                    {
                        //transition
                        bufferBarriers[bufferBarrierCount].AccessFlagsBefore = output.buffer->currentAccess;
                        bufferBarriers[bufferBarrierCount].buffer = output.buffer->buffer;
                        bufferBarriers[bufferBarrierCount].previousQueue = output.buffer->currentFamily;
                        bufferBarriers[bufferBarrierCount].nextQueue = RHI::QueueFamily::Compute;
                        bufferBarriers[bufferBarrierCount].offset = output.buffer->offset;
                        bufferBarriers[bufferBarrierCount].size = output.buffer->size;
                        output.buffer->currentFamily = RHI::QueueFamily::Compute;
                        output.buffer->currentAccess = bufferBarriers[bufferBarrierCount].AccessFlagsAfter;
                        bufferBarrierCount++;
                    }
                }
                currentList->PipelineBarrier(RHI::PipelineStage::TOP_OF_PIPE_BIT, pass->stage, bufferBarrierCount, bufferBarriers, barrierCount, barriers);
                if (pass->pso) currentList->SetPipelineState(pass->pso);
                if (attachmentCount) currentList->BeginRendering(&rbDesc);
                pass->pass_fn(currentList);
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
                    barriers[barrierCount].newLayout = InputLayout(input.usage);
                    barriers[barrierCount].AccessFlagsAfter = InputAccess(input.usage);
                    if (
                        input.texture->current_layout != barriers[barrierCount].newLayout ||
                        input.texture->currentAccess != barriers[barrierCount].AccessFlagsAfter ||
                        input.texture->currentFamily != RHI::QueueFamily::Compute
                        )
                    {
                        //temporary
                        RHI::SubResourceRange range;
                        range.FirstArraySlice = input.texture->IsArray ? input.texture->arraySlice : 0;
                        range.imageAspect = RHI::Aspect::COLOR_BIT;
                        range.IndexOrFirstMipLevel = input.texture->mipSlice;
                        range.NumArraySlices = 1;
                        range.NumMipLevels = 1;
                        //transition
                        barriers[barrierCount].AccessFlagsBefore = input.texture->currentAccess;
                        barriers[barrierCount].oldLayout = input.texture->current_layout;
                        barriers[barrierCount].texture = input.texture->texture;
                        barriers[barrierCount].subresourceRange = range;
                        barriers[barrierCount].previousQueue = input.texture->currentFamily;
                        barriers[barrierCount].nextQueue = RHI::QueueFamily::Compute;
                        input.texture->currentFamily = RHI::QueueFamily::Compute;
                        input.texture->current_layout = barriers[barrierCount].newLayout;
                        input.texture->currentAccess = barriers[barrierCount].AccessFlagsAfter;
                        barrierCount++;
                    }
                }
                for (auto& output : pass->outputs)
                {
                    barriers[barrierCount].newLayout = OutputLayout(output.usage);
                    barriers[barrierCount].AccessFlagsAfter = OutputAccess(output.usage);
                    if (
                        output.texture->current_layout != barriers[barrierCount].newLayout ||
                        output.texture->currentAccess != barriers[barrierCount].AccessFlagsAfter ||
                        output.texture->currentFamily != RHI::QueueFamily::Compute
                        )
                    {
                        //temporary
                        RHI::SubResourceRange range;
                        range.FirstArraySlice = output.texture->IsArray ? output.texture->arraySlice : 0;
                        range.imageAspect = RHI::Aspect::COLOR_BIT;
                        range.IndexOrFirstMipLevel = output.texture->mipSlice;
                        range.NumArraySlices = 1;
                        range.NumMipLevels = 1;
                        //transition
                        barriers[barrierCount].AccessFlagsBefore = output.texture->currentAccess;
                        barriers[barrierCount].oldLayout = output.texture->current_layout;
                        barriers[barrierCount].texture = output.texture->texture;
                        barriers[barrierCount].subresourceRange = range;
                        barriers[barrierCount].previousQueue = output.texture->currentFamily;
                        barriers[barrierCount].nextQueue = RHI::QueueFamily::Compute;
                        output.texture->currentFamily = RHI::QueueFamily::Compute;
                        output.texture->current_layout = barriers[barrierCount].newLayout;
                        output.texture->currentAccess = barriers[barrierCount].AccessFlagsAfter;
                        barrierCount++;
                    }
                }
                
                for (auto& input : pass->bufferInputs) 
                {
                    bufferBarriers[bufferBarrierCount].AccessFlagsAfter = InputAccess(input.usage);
                    if (
                        input.buffer->currentAccess != bufferBarriers[bufferBarrierCount].AccessFlagsAfter ||
                        input.buffer->currentFamily != RHI::QueueFamily::Compute
                        )
                    {
                        //transition
                        bufferBarriers[bufferBarrierCount].AccessFlagsBefore = input.buffer->currentAccess;
                        bufferBarriers[bufferBarrierCount].buffer = input.buffer->buffer;
                        bufferBarriers[bufferBarrierCount].previousQueue = input.buffer->currentFamily;
                        bufferBarriers[bufferBarrierCount].nextQueue = RHI::QueueFamily::Compute;
                        bufferBarriers[bufferBarrierCount].offset = input.buffer->offset;
                        bufferBarriers[bufferBarrierCount].size = input.buffer->size;
                        input.buffer->currentFamily = RHI::QueueFamily::Compute;
                        input.buffer->currentAccess = bufferBarriers[bufferBarrierCount].AccessFlagsAfter;
                        bufferBarrierCount++;
                    }
                }
                for (auto& output : pass->bufferOutputs)
                {
                    bufferBarriers[bufferBarrierCount].AccessFlagsAfter = OutputAccess(output.usage);
                    if (
                        output.buffer->currentAccess != bufferBarriers[bufferBarrierCount].AccessFlagsAfter ||
                        output.buffer->currentFamily != RHI::QueueFamily::Compute
                        )
                    {
                        //transition
                        bufferBarriers[bufferBarrierCount].AccessFlagsBefore = output.buffer->currentAccess;
                        bufferBarriers[bufferBarrierCount].buffer = output.buffer->buffer;
                        bufferBarriers[bufferBarrierCount].previousQueue = output.buffer->currentFamily;
                        bufferBarriers[bufferBarrierCount].nextQueue = RHI::QueueFamily::Compute;
                        bufferBarriers[bufferBarrierCount].offset = output.buffer->offset;
                        bufferBarriers[bufferBarrierCount].size = output.buffer->size;
                        output.buffer->currentFamily = RHI::QueueFamily::Compute;
                        output.buffer->currentAccess = bufferBarriers[bufferBarrierCount].AccessFlagsAfter;
                        bufferBarrierCount++;
                    }
                }
                currentList->PipelineBarrier(RHI::PipelineStage::TOP_OF_PIPE_BIT, RHI::PipelineStage::COMPUTE_SHADER_BIT, bufferBarrierCount, bufferBarriers, barrierCount, barriers);
                currentList->SetComputePipeline(pass->computePipeline);
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
        std::vector<std::tuple<RGTexture*, PassType, uint32_t, void*>> readyOutputs;
        std::vector<std::tuple<RGBuffer*, PassType, uint32_t, void*>> readyBufferOutputs;

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
                        [input, &fenceVal](const std::tuple<RGTexture*, PassType, uint64_t, void*>& output)
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
                        [input, &fenceVal](const std::tuple<RGBuffer*, PassType, uint64_t, void*>& output)
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
                    if (pass->dsOutput.texture)
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
                        [input, &fenceVal](const std::tuple<RGTexture*, PassType, uint64_t, void*>& output)
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
                        [input, &fenceVal](const std::tuple<RGBuffer*, PassType, uint64_t, void*>& output)
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
            if (!(levelTransitionIndices.size() && (levelTransitionIndices.end() - 1)->first == passesSortedAndFence.size()))
                levelTransitionIndices.push_back({ passesSortedAndFence.size(),PassAction::Wait });
            if (!(computeLevelTransitionIndices.size() && (computeLevelTransitionIndices.end() - 1)->first == computePassesSortedAndFence.size()))
                computeLevelTransitionIndices.push_back({ computePassesSortedAndFence.size(),PassAction::Wait });
        }
        (levelTransitionIndices.end() - 1)->second = (PassAction)0;
        (computeLevelTransitionIndices.end() - 1)->second = (PassAction)0;
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
        numGFXCmdLists = levelTransitionIndices.size();
        numComputeCmdLists = computeLevelTransitionIndices.size();
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
                RendererBase::commandAllocators[RendererBase::currentFrameIndex], &cmdLists[i].list);
        }
        dirty = false;
    }

}
