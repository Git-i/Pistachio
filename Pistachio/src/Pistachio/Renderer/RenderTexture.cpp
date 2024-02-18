#include "ptpch.h"
#include "RendererBase.h"
#include "RenderTexture.h"

namespace Pistachio
{
    RenderTexture* RenderTexture::Create(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format)
    {
        RenderTexture* returnVal = new RenderTexture;
        returnVal->CreateStack(width,height,mipLevels,format);
        return returnVal;
    }
    void RenderTexture::CreateStack(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format)
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
        desc.usage = RHI::TextureUsage::ColorAttachment | RHI::TextureUsage::SampledImage;
        RHI::AutomaticAllocationInfo allocInfo;
        allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
        RendererBase::device->CreateTexture(&desc, &m_ID, nullptr, nullptr, &allocInfo, 0, RHI::ResourceType::Automatic);
        RHI::SubResourceRange range;
        range.FirstArraySlice = 0;
        range.imageAspect = RHI::Aspect::COLOR_BIT;
        range.IndexOrFirstMipLevel = 0;
        range.NumArraySlices = 1;
        range.NumMipLevels = mipLevels;

        RHI::TextureViewDesc viewDesc;
        viewDesc.format = format;
        viewDesc.range = range;
        viewDesc.texture = m_ID.Get();
        viewDesc.type = RHI::TextureViewType::Texture2D;
        RendererBase::device->CreateTextureView(&viewDesc, &m_view);
        
        RHI::RenderTargetViewDesc rtDesc;
        rtDesc.arraySlice = 0;
        rtDesc.format = format;
        rtDesc.TextureArray = 0;
        rtDesc.textureMipSlice = 0;
        RTView = RendererBase::CreateRenderTargetView(m_ID.Get(), &rtDesc);
	}
    RHI::Format RenderTexture::GetFormat() const{return m_format;}
    uint32_t RenderTexture::GetWidth()  const{ return m_width; }
    uint32_t RenderTexture::GetHeight() const{ return m_height; }
    RenderCubeMap* RenderCubeMap::Create(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format, RHI::TextureUsage extraUsage)
    {
        RenderCubeMap* returnVal = new RenderCubeMap;
        returnVal->CreateStack(width, height, mipLevels, format,extraUsage);
        return returnVal;
    }
    void RenderCubeMap::CreateStack(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format, RHI::TextureUsage extraUsage)
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
        RendererBase::device->CreateTexture(&desc, &m_ID, nullptr, nullptr, &allocInfo, 0, RHI::ResourceType::Automatic);
        RHI::SubResourceRange range;
        range.FirstArraySlice = 0;
        range.imageAspect = RHI::Aspect::COLOR_BIT;
        range.IndexOrFirstMipLevel = 0;
        range.NumArraySlices = 6;
        range.NumMipLevels = mipLevels;

        RHI::TextureViewDesc viewDesc;
        viewDesc.format = format;
        viewDesc.range = range;
        viewDesc.texture = m_ID.Get();
        viewDesc.type = RHI::TextureViewType::TextureCube;
        RendererBase::device->CreateTextureView(&viewDesc, &m_view);

        for (uint32_t i = 0; i < 6; i++)
        {
            RHI::RenderTargetViewDesc rtDesc;
            rtDesc.arraySlice = i;
            rtDesc.format = format;
            rtDesc.TextureArray = true;
            rtDesc.textureMipSlice = 0;
            RTViews[i] = RendererBase::CreateRenderTargetView(m_ID.Get(), &rtDesc);
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
        barrier.texture = m_ID.Get();
        if(list)
        list->PipelineBarrier(RHI::PipelineStage::TOP_OF_PIPE_BIT, RHI::PipelineStage::ALL_GRAPHICS_BIT, 0, 0, 1, &barrier);
        else
            RendererBase::mainCommandList->PipelineBarrier(RHI::PipelineStage::TOP_OF_PIPE_BIT, RHI::PipelineStage::ALL_GRAPHICS_BIT, 0, 0, 1, &barrier);
    }
    DepthTexture* DepthTexture::Create(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format)
    {
        DepthTexture* returnVal = new DepthTexture;
        returnVal->CreateStack(width, height, mipLevels, format);
        return returnVal;
    }
    void DepthTexture::CreateStack(uint32_t width, uint32_t height, uint32_t mipLevels, RHI::Format format)
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
        RendererBase::device->CreateTexture(&desc, &m_ID, nullptr, nullptr, &allocInfo, 0, RHI::ResourceType::Automatic);
        RHI::SubResourceRange range;
        range.FirstArraySlice = 0;
        range.imageAspect = RHI::Aspect::DEPTH_BIT;
        range.IndexOrFirstMipLevel = 0;
        range.NumArraySlices = 1;
        range.NumMipLevels = mipLevels;

        RHI::TextureViewDesc viewDesc;
        viewDesc.format = format;
        viewDesc.range = range;
        viewDesc.texture = m_ID.Get();
        viewDesc.type = RHI::TextureViewType::Texture2D;
        RendererBase::device->CreateTextureView(&viewDesc, &m_view);

        RHI::DepthStencilViewDesc dsDesc;
        dsDesc.arraySlice = 0;
        dsDesc.format = format;
        dsDesc.TextureArray = 0;
        dsDesc.textureMipSlice = 0;
        dsDesc.TextureArray = 0;
        DSView = RendererBase::CreateDepthStencilView(m_ID.Get(), &dsDesc);
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
