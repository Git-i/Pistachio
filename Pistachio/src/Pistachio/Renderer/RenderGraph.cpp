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
        for (uint32_t i = 0; i < numCmdLists; i++)
        {
            cmdLists[i].list->Release();
        }
        delete[] cmdLists;
    }
    RenderGraph::RenderGraph(uint32_t numCmdLists)
    {
        PT_CORE_ASSERT(numCmdLists < 10);
        cmdLists = new RGCommandList[numCmdLists];
        this->numCmdLists = numCmdLists;
        for (uint32_t i = 0; i < numCmdLists; i++)
        {
            RendererBase::device->CreateCommandList(
                RHI::CommandListType::Direct,
                RendererBase::commandAllocators[RendererBase::currentFrameIndex], &cmdLists[i].list);
        }
    }
    void RenderGraph::SubmitToQueue()
    {
        Internal_ID lists[10];
        for (uint32_t i = 0; i < numCmdLists; i++)
        {
            cmdLists[i].list->End();
            lists[i] = cmdLists[i].list->ID;
        }
        RendererBase::directQueue->ExecuteCommandLists(lists, numCmdLists);
        
    }
    void RenderGraph::NewFrame()
    {
        for (uint32_t i = 0; i < numCmdLists; i++)
        {
            cmdLists[i].list->Begin(RendererBase::commandAllocators[RendererBase::currentFrameIndex]);
        }
    }
    RGTexture* RenderGraph::CreateTexture(RenderTexture* texture)
    {
        auto tex = textures.emplace_back(new RGTexture(texture->m_ID.Get(), RHI::ResourceLayout::UNDEFINED, 0, false, 0));
        tex->rtvHandle = texture->RTView;
        return tex;
    }
    RGTexture* RenderGraph::CreateTexture(DepthTexture* texture)
    {
        auto tex = textures.emplace_back(new RGTexture(texture->m_ID.Get(), RHI::ResourceLayout::UNDEFINED, 0, false, 0));
        tex->dsvHandle = texture->DSView;
        return tex;
    }
    RGTexture* RenderGraph::CreateTexture(RenderCubeMap* texture, uint32_t cubeIndex)
    {
        auto tex = textures.emplace_back(new RGTexture(texture->m_ID.Get(), RHI::ResourceLayout::UNDEFINED, 0, true, cubeIndex));
        tex->rtvHandle = texture->RTViews[cubeIndex];
        return tex;
    }
    RenderPass& RenderGraph::AddPass(RHI::PipelineStage stage, const char* name)
    {
        auto& pass = passes.emplace_back();
        pass.stage = stage;
        return pass;
    }
    RGTexture* RenderGraph::CreateTexture(Pistachio::Texture* texture, uint32_t mipSlice, bool isArray , uint32_t arraySlice, RHI::ResourceLayout layout)
    {
        auto tex = textures.emplace_back(new RGTexture(texture->m_ID.Get(), layout, mipSlice, isArray, arraySlice));
        return tex;
    }
    RGTexture* RenderGraph::CreateTexture(RHI::Texture* texture, uint32_t mipSlice, bool isArray, uint32_t arraySlice, RHI::ResourceLayout layout)
    {
        auto tex = textures.emplace_back(new RGTexture(texture, layout,mipSlice,isArray,arraySlice));
        return tex;
    }
    void RenderPass::SetShader(Shader* shader)
    {
        pso = shader->GetCurrentPipeline();
    }
    void RenderPass::AddColorInput(AttachmentInfo* info)
    {
        inputs.push_back(*info);
    }
    void RenderPass::AddColorOutput(AttachmentInfo* info)
    {
        outputs.push_back(*info);
    }
    void RenderPass::SetPassArea(const RHI::Area2D& _area)
    {
        area = _area;
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
        default:
            break;
        }
    }
    void RenderGraph::Execute()
    {
        /*every element of levelTransitionIndices tells what index the next level starts
        * eg.
        * levelTransitionIndices = {2,5,9} means
        * 0 - l0
        * 1 - l0
        * 2 - l1
        * 3 - l1
        * 4 - l1
        * 5 - l2
        * 6 - l2
        * 7 - l2
        * 8 - l2
        * with 9 passes total
        */
        //for every level (we can execute async)
        SortPasses();
        uint32_t numLevelsPerList = levelTransitionIndices.size() / numCmdLists;
        for (uint32_t i = 0; i < levelTransitionIndices.size(); i++)
        {
            uint32_t startingPass = i ? levelTransitionIndices[i - 1] : 0;
            uint32_t numPasses = levelTransitionIndices[i] - startingPass;
            uint32_t listIndex = std::min(i/numLevelsPerList, numCmdLists);
            RGCommandList& listToUse = cmdLists[listIndex];
            for (uint32_t j = startingPass; j < startingPass+numPasses; j++)
            {
                //every pass in the current level
                //we still have to manage transitions
                RenderPass* pass = passesSorted[j];
                RHI::TextureMemoryBarrier* barriers = new RHI::TextureMemoryBarrier[pass->inputs.size() + pass->outputs.size() + (pass->dsOutput.texture ? 1 : 0)];//alloca??
                uint32_t barrierCount = 0;
                RHI::RenderingBeginDesc rbDesc;
                rbDesc.numColorAttachments = pass->outputs.size();
                rbDesc.pDepthStencilAttachment = nullptr;
                rbDesc.renderingArea = pass->area;
                RHI::RenderingAttachmentDesc* attachments = new RHI::RenderingAttachmentDesc[pass->outputs.size() + (pass->dsOutput.texture ? 1 : 0)];
                rbDesc.pColorAttachments = attachments;
                uint32_t attachmentCount = 0;
                for (auto& input : pass->inputs)
                {
                    //assuming all inputs are supposed to be in shader_read
                    barriers[barrierCount].newLayout = InputLayout(input.usage);

                    if (input.texture->current_layout != barriers[barrierCount].newLayout)
                    {
                        //temporary
                        RHI::SubResourceRange range;
                        range.FirstArraySlice = input.texture->IsArray ? input.texture->arraySlice : 0;
                        range.imageAspect = RHI::Aspect::COLOR_BIT;
                        range.IndexOrFirstMipLevel = input.texture->mipSlice;
                        range.NumArraySlices = 1;
                        range.NumMipLevels = 1;
                        //transition
                        barriers[barrierCount].AccessFlagsAfter = InputAccess(input.usage);//maybe?
                        barriers[barrierCount].AccessFlagsBefore = RHI::ResourceAcessFlags::NONE;
                        barriers[barrierCount].oldLayout = input.texture->current_layout;
                        barriers[barrierCount].texture =   input.texture->texture;
                        barriers[barrierCount].subresourceRange = range;
                        input.texture->current_layout = barriers[barrierCount].newLayout;
                        barrierCount++;
                    }
                }
                for (auto& output : pass->outputs)
                {
                    if (output.usage == AttachmentUsage::Graphics)
                    {
                        attachments[attachmentCount].clearColor = { 0,0,0,0 };
                        attachments[attachmentCount].loadOp = output.loadOp;
                        attachments[attachmentCount].storeOp = RHI::StoreOp::Store; // if !output->inputUsers StoreOp::DontCare
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
                    if (output.texture->current_layout != barriers[barrierCount].newLayout)
                    {
                        //temporary
                        RHI::SubResourceRange range;
                        range.FirstArraySlice = output.texture->IsArray ? output.texture->arraySlice: 0 ;
                        range.imageAspect = RHI::Aspect::COLOR_BIT;
                        range.IndexOrFirstMipLevel = output.texture->mipSlice;
                        range.NumArraySlices = 1;
                        range.NumMipLevels = 1;
                        //transition
                        barriers[barrierCount].AccessFlagsAfter = OutputAccess(output.usage);
                        barriers[barrierCount].AccessFlagsBefore = RHI::ResourceAcessFlags::NONE;
                        barriers[barrierCount].oldLayout = output.texture->current_layout;
                        barriers[barrierCount].texture =   output.texture->texture;
                        barriers[barrierCount].subresourceRange = range;
                        output.texture->current_layout = barriers[barrierCount].newLayout;
                        barrierCount++;
                    }
                }
                if (pass->dsOutput.texture)
                {
                    attachments[attachmentCount].clearColor = { 1,1,1,1};
                    attachments[attachmentCount].loadOp = RHI::LoadOp::Clear;//todo
                    attachments[attachmentCount].storeOp = RHI::StoreOp::Store; // if !output->inputUsers StoreOp::DontCare
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
                    if (pass->dsOutput.texture->current_layout != RHI::ResourceLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
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
                        barriers[barrierCount].oldLayout = pass->dsOutput.texture->current_layout;
                        barriers[barrierCount].texture = pass->dsOutput.texture->texture;
                        barriers[barrierCount].subresourceRange = range;
                        barrierCount++;
                    }
                }
                listToUse.listMutex.lock();
                listToUse.list->PipelineBarrier(RHI::PipelineStage::TOP_OF_PIPE_BIT, pass->stage, 0, nullptr, barrierCount, barriers);
                if(pass->pso) listToUse.list->SetPipelineState(pass->pso);
                if(attachmentCount) listToUse.list->BeginRendering(&rbDesc);
                pass->pass_fn(listToUse.list);
                if(attachmentCount) listToUse.list->EndRendering();
                listToUse.listMutex.unlock();
                delete[] barriers;
                delete[] attachments;
            }
        }
        levelTransitionIndices.clear();
        passesSorted.clear();
    }

    void RenderGraph::SortPasses()
    {
        //passes have levels, each pass in a level can be executed async
        //and passes with higher level cannot execute till passes of lower level are done
        std::vector<RenderPass*> passesLeft;
        for (auto& pass : passes) { passesLeft.push_back(&pass); }

        std::vector<RGTexture*> readyOutputs;
        uint32_t intendedOutputIndex = 0;
        uint32_t readyOutputIndex = 0;
        while (passesLeft.size())
        {
            std::vector<RenderPass*> passesWithFurtherLevel;
            for (uint32_t i = 0; i < passesLeft.size(); i++)
            {
                RenderPass* pass = passesLeft[i];
                //if all a pass inputs are in the ready Outputs,pass belong to current level
                bool currLevel = true;
                for (auto& input : pass->inputs)
                {
                    if (auto pos = std::find(readyOutputs.begin(), readyOutputs.begin() + readyOutputIndex, input.texture);
                        pos == readyOutputs.begin() + readyOutputIndex)
                    {
                        //one input isn't ready, its not for this level
                        passesWithFurtherLevel.push_back(pass);
                        currLevel = false;
                        break;
                    }
                }
                if (currLevel)
                {
                    passesSorted.push_back(pass);
                    for (auto output : pass->outputs)
                    {
                        intendedOutputIndex++;
                        readyOutputs.push_back(output.texture);
                    }
                    if (pass->dsOutput.texture) 
                    {
                        intendedOutputIndex++; readyOutputs.push_back(pass->dsOutput.texture);
                    };
                }
            }
            readyOutputIndex = intendedOutputIndex;
            passesLeft = std::move(passesWithFurtherLevel);
            levelTransitionIndices.push_back(passesSorted.size());
        }


    }

}
