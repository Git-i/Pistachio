#include "Pistachio/Core/Log.h"
#include "ptpch.h"
#include "RendererBase.h"
#include "RenderTexture.h"

namespace Pistachio
{
    RenderTexture* RenderTexture::Create(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format PT_DEBUG_REGION(, const char* name))
    {
        RenderTexture* returnVal = new RenderTexture;
        returnVal->CreateStack(width,height,mipLevels,format PT_DEBUG_REGION(, name));
        return returnVal;
    }
    void RenderTexture::CreateStack(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format PT_DEBUG_REGION(, const char* name))
	{
        m_width = width;
        m_height = height;
        m_mipLevels = mipLevels;
        m_format = format;
        RHI::TextureDesc desc{};
        desc.depthOrArraySize = 1;
        desc.height = height;
        desc.width = width;
        desc.mipLevels = mipLevels;
        desc.mode = RHI::TextureTilingMode::Optimal;
        desc.optimizedClearValue = nullptr;
        desc.format = format;
        desc.sampleCount = 1;
        desc.type = RHI::TextureType::Texture2D;
        desc.usage = RHI::TextureUsage::ColorAttachment | RHI::TextureUsage::SampledImage | RHI::TextureUsage::CopySrc;
        RHI::AutomaticAllocationInfo allocInfo;
        allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
        m_ID = RendererBase::device->CreateTexture(desc, nullptr, nullptr, &allocInfo, 0, RHI::ResourceType::Automatic).value();
        PT_DEBUG_REGION(m_ID->SetName(name));
        RHI::SubResourceRange range;
        range.FirstArraySlice = 0;
        range.imageAspect = RHI::Aspect::COLOR_BIT;
        range.IndexOrFirstMipLevel = 0;
        range.NumArraySlices = 1;
        range.NumMipLevels = mipLevels;

        RHI::TextureViewDesc viewDesc;
        viewDesc.format = format;
        viewDesc.range = range;
        viewDesc.texture = m_ID;
        viewDesc.type = RHI::TextureViewType::Texture2D;
        m_view = RendererBase::device->CreateTextureView(viewDesc).value();
        
        RHI::RenderTargetViewDesc rtDesc;
        rtDesc.arraySlice = 0;
        rtDesc.format = format;
        rtDesc.TextureArray = 0;
        rtDesc.textureMipSlice = 0;
        RTView = RendererBase::CreateRenderTargetView(m_ID, rtDesc);
	}
    RHI::Format RenderTexture::GetFormat() const{return m_format;}
    uint32_t RenderTexture::GetWidth()  const{ return m_width; }
    uint32_t RenderTexture::GetHeight() const{ return m_height; }
    RenderCubeMap* RenderCubeMap::Create(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format PT_DEBUG_REGION(, const char* name), RHI::TextureUsage extraUsage)
    {
        RenderCubeMap* returnVal = new RenderCubeMap;
        returnVal->CreateStack(width, height, mipLevels, format PT_DEBUG_REGION(, name) ,extraUsage );
        return returnVal;
    }
    void RenderCubeMap::CreateStack(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format PT_DEBUG_REGION(, const char* name), RHI::TextureUsage extraUsage)
    {
        m_width = width;
        m_height = height;
        m_mipLevels = mipLevels;
        m_format = format;
        RHI::TextureDesc desc{};
        desc.depthOrArraySize = 6;
        desc.height = height;
        desc.width = width;
        desc.mipLevels = mipLevels;
        desc.mode = RHI::TextureTilingMode::Optimal;
        desc.optimizedClearValue = nullptr;
        desc.format = format;
        desc.sampleCount = 1;
        desc.type = RHI::TextureType::Texture2D;
        desc.usage = RHI::TextureUsage::ColorAttachment | RHI::TextureUsage::SampledImage | RHI::TextureUsage::CubeMap | extraUsage;
        RHI::AutomaticAllocationInfo allocInfo;
        allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
        m_ID = RendererBase::device->CreateTexture(desc, nullptr, nullptr, &allocInfo, 0, RHI::ResourceType::Automatic).value();
        PT_DEBUG_REGION(m_ID->SetName(name));
        RHI::SubResourceRange range;
        range.FirstArraySlice = 0;
        range.imageAspect = RHI::Aspect::COLOR_BIT;
        range.IndexOrFirstMipLevel = 0;
        range.NumArraySlices = 6;
        range.NumMipLevels = mipLevels;

        RHI::TextureViewDesc viewDesc;
        viewDesc.format = format;
        viewDesc.range = range;
        viewDesc.texture = m_ID;
        viewDesc.type = RHI::TextureViewType::TextureCube;
        m_view = RendererBase::device->CreateTextureView(viewDesc).value();

        for (uint32_t i = 0; i < 6; i++)
        {
            RHI::RenderTargetViewDesc rtDesc;
            rtDesc.arraySlice = i;
            rtDesc.format = format;
            rtDesc.TextureArray = true;
            rtDesc.textureMipSlice = 0;
            RTViews[i] = RendererBase::CreateRenderTargetView(m_ID, rtDesc);
        }
    }
    void RenderCubeMap::SwitchToRenderTargetMode(RHI::GraphicsCommandList* list)
    {
        //todo
    }
    void RenderCubeMap::SwitchToShaderUsageMode(RHI::GraphicsCommandList* list)
    {
        RHI::TextureMemoryBarrier barrier;
        barrier.AccessFlagsBefore = RHI::ResourceAcessFlags::NONE;
        barrier.AccessFlagsAfter = RHI::ResourceAcessFlags::SHADER_READ;
        barrier.oldLayout = RHI::ResourceLayout::UNDEFINED;
        barrier.newLayout = RHI::ResourceLayout::SHADER_READ_ONLY_OPTIMAL;
        RHI::SubResourceRange range;
        range.FirstArraySlice = 0;
        range.imageAspect = RHI::Aspect::COLOR_BIT;
        range.IndexOrFirstMipLevel = 0;
        range.NumArraySlices = 6;
        range.NumMipLevels = m_mipLevels;
        barrier.subresourceRange = range;
        barrier.texture = m_ID;
        if(list)
        list->PipelineBarrier(RHI::PipelineStage::TOP_OF_PIPE_BIT, RHI::PipelineStage::ALL_GRAPHICS_BIT, {}, {&barrier,1});
        else
            RendererBase::mainCommandList->PipelineBarrier(RHI::PipelineStage::TOP_OF_PIPE_BIT, RHI::PipelineStage::ALL_GRAPHICS_BIT, {}, {&barrier,1});
    }
    DepthTexture* DepthTexture::Create(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format PT_DEBUG_REGION(,const char* name))
    {
        DepthTexture* returnVal = new DepthTexture;
        returnVal->CreateStack(width, height, mipLevels, format PT_DEBUG_REGION(, name));
        return returnVal;
    }
    void DepthTexture::CreateStack(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format PT_DEBUG_REGION(,const char* name))
    {
        m_width = width;
        m_height = height;
        m_mipLevels = mipLevels;
        m_format = format;
        RHI::TextureDesc desc{};
        desc.depthOrArraySize = 1;
        desc.height = height;
        desc.width = width;
        desc.mipLevels = mipLevels;
        desc.mode = RHI::TextureTilingMode::Optimal;
        desc.optimizedClearValue = nullptr;
        desc.format = format;
        desc.sampleCount = 1;
        desc.type = RHI::TextureType::Texture2D;
        desc.usage = RHI::TextureUsage::DepthStencilAttachment | RHI::TextureUsage::SampledImage;
        RHI::AutomaticAllocationInfo allocInfo;
        allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
        m_ID = RendererBase::device->CreateTexture(desc, nullptr, nullptr, &allocInfo, 0, RHI::ResourceType::Automatic).value();
        PT_DEBUG_REGION(m_ID->SetName(name));
        RHI::SubResourceRange range;
        range.FirstArraySlice = 0;
        range.imageAspect = RHI::Aspect::DEPTH_BIT;
        range.IndexOrFirstMipLevel = 0;
        range.NumArraySlices = 1;
        range.NumMipLevels = mipLevels;

        RHI::TextureViewDesc viewDesc;
        viewDesc.format = format;
        viewDesc.range = range;
        viewDesc.texture = m_ID;
        viewDesc.type = RHI::TextureViewType::Texture2D;
        m_view = RendererBase::device->CreateTextureView(viewDesc).value();

        RHI::DepthStencilViewDesc dsDesc;
        dsDesc.arraySlice = 0;
        dsDesc.format = format;
        dsDesc.TextureArray = 0;
        dsDesc.textureMipSlice = 0;
        dsDesc.TextureArray = 0;
        DSView = RendererBase::CreateDepthStencilView(m_ID, dsDesc);
    }
    RHI::Format DepthTexture::GetFormat() const
    {
        return m_format;
    }
    uint32_t DepthTexture::GetWidth() const
    {
        return m_width;
    }
    uint32_t DepthTexture::GetHeight() const
    {
        return  m_height;
    }
}
