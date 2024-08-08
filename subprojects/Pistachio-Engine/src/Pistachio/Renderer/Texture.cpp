#include "Pistachio/Core/Log.h"
#include "ptpch.h"
#include "Texture.h"
#include "Util/FormatUtils.h"
#include "stb_image.h"
#include "RendererBase.h"
namespace Pistachio
{
    void Texture2D::CreateTexture(void* data, TextureFlags flags)
    {
        PT_PROFILE_FUNCTION();
        RHI::TextureDesc desc{};
        desc.depthOrArraySize = 1;
        desc.height = m_Height;
        desc.width = m_Width;
        desc.mipLevels = 1; //add mip-mapping ??
        desc.mode = RHI::TextureTilingMode::Optimal;
        desc.optimizedClearValue = nullptr;
        desc.format = m_format;
        desc.sampleCount = 1;
        desc.type = RHI::TextureType::Texture2D;
        desc.usage = RHI::TextureUsage::CopyDst | RHI::TextureUsage::SampledImage;
        desc.usage |= ((flags & TextureFlags::Compute) != TextureFlags::None) ? RHI::TextureUsage::StorageImage : RHI::TextureUsage::None;
        RHI::AutomaticAllocationInfo allocInfo;
        allocInfo.access_mode = RHI::AutomaticAllocationCPUAccessMode::None;
        m_ID = RendererBase::device->CreateTexture(desc, nullptr, nullptr, &allocInfo, 0, RHI::ResourceType::Automatic).value();
        RHI::SubResourceRange range;
        range.FirstArraySlice = 0;
        range.imageAspect = RHI::Aspect::COLOR_BIT;
        range.IndexOrFirstMipLevel = 0;
        range.NumArraySlices = 1;
        range.NumMipLevels = 1;
        
        if (data)
        {
            RHI::TextureMemoryBarrier barrier;
            barrier.AccessFlagsAfter = RHI::ResourceAcessFlags::TRANSFER_WRITE;
            barrier.AccessFlagsBefore = RHI::ResourceAcessFlags::NONE;
            barrier.newLayout = RHI::ResourceLayout::TRANSFER_DST_OPTIMAL;
            barrier.oldLayout = RHI::ResourceLayout::UNDEFINED;
            barrier.previousQueue = RHI::QueueFamily::Graphics;
            barrier.nextQueue = RHI::QueueFamily::Graphics;
            barrier.subresourceRange = range;
            barrier.texture = m_ID;
            RendererBase::stagingCommandList->PipelineBarrier(
                RHI::PipelineStage::TOP_OF_PIPE_BIT,
                RHI::PipelineStage::TRANSFER_BIT,
                {},
                {&barrier,1});
            RendererBase::PushTextureUpdate(m_ID, m_Width * m_Height * RHI::Util::GetFormatBPP(m_format), data, &range, {m_Width, m_Height,1}, {0,0,0},m_format); //todo image sizes
            barrier.AccessFlagsBefore = RHI::ResourceAcessFlags::TRANSFER_WRITE;
            barrier.AccessFlagsAfter = RHI::ResourceAcessFlags::SHADER_READ;
            barrier.newLayout = RHI::ResourceLayout::SHADER_READ_ONLY_OPTIMAL;
            barrier.oldLayout = RHI::ResourceLayout::TRANSFER_DST_OPTIMAL;
            RendererBase::stagingCommandList->PipelineBarrier(
                RHI::PipelineStage::TRANSFER_BIT,
                RHI::PipelineStage::FRAGMENT_SHADER_BIT,
                {},
                {&barrier, 1});
        }
        RHI::TextureViewDesc viewDesc;
        viewDesc.format = m_format;
        viewDesc.range = range;
        viewDesc.texture = m_ID;
        viewDesc.type = RHI::TextureViewType::Texture2D;
        m_view = RendererBase::device->CreateTextureView(viewDesc).value();
    }
    RHI::Format Texture2D::GetFormat() const
    {
        PT_PROFILE_FUNCTION();
        return m_format;
    }
    unsigned int Texture2D::GetHeight() const
    {
        PT_PROFILE_FUNCTION();
        return m_Height;
    }

    unsigned int Texture2D::GetWidth() const
    {
        PT_PROFILE_FUNCTION();
        return m_Width;
    }

    void Texture2D::Bind(int slot) const
    {
        PT_PROFILE_FUNCTION();
    }

    Texture2D* Texture2D::Create(const char* path PT_DEBUG_REGION(, const char* name), RHI::Format format, TextureFlags flags)
    {
        PT_PROFILE_FUNCTION();
        Texture2D* result = new Texture2D;
        result->CreateStack(path, format PT_DEBUG_REGION(, name));
        return result;
    }
    void Texture2D::CreateStack(const char* path, RHI::Format format PT_DEBUG_REGION(, const char* name), TextureFlags flags )
    {
        PT_PROFILE_FUNCTION();
        int Width, Height, nChannels;
        void* data;
        if (format == RHI::Format::R16G16B16A16_FLOAT || format == RHI::Format::R32G32B32A32_FLOAT )
        {
            PT_PROFILE_SCOPE("stbi_loadf");
            data = stbi_loadf(path, &Width, &Height, &nChannels, 4);
            PT_CORE_ASSERT(data);
        }
        else
        {
            PT_PROFILE_SCOPE("stbi_load");
            data = stbi_load(path, &Width, &Height, &nChannels, 4);
            PT_CORE_ASSERT(data);
        }
        m_Width = Width;
        m_Height = Height;
        m_format = format;
        CreateTexture(data, flags);
        PT_DEBUG_REGION(if(m_ID.IsValid()) m_ID->SetName(name)); 
        stbi_image_free(data);
    }
    void Texture2D::CreateStack(uint32_t width, uint32_t height, RHI::Format format, void* data PT_DEBUG_REGION(, const char* name),TextureFlags flags)
    {
        PT_PROFILE_FUNCTION();
        m_Width = width;
        m_Height = height;
        m_format = format;
        CreateTexture(data, flags);
        PT_DEBUG_REGION(if(m_ID.IsValid()) m_ID->SetName(name));
    }
    void Texture2D::CopyIntoRegion(Texture2D& source, unsigned int location_x, unsigned int location_y, unsigned int src_left, unsigned int src_right, unsigned int src_up, unsigned int src_down, unsigned int mipSlice, unsigned int arraySlice)
    {
        //PT_PROFILE_FUNCTION();
        //ID3D11Resource* pDstResource;
        //ID3D11Resource* pSrcResource;
        //if (m_bHasView)
        //    ((ID3D11ShaderResourceView*)m_ID.Get())->GetResource(&pDstResource);
        //else
        //    pDstResource = (ID3D11Resource*)(m_ID.Get());
        //((ID3D11ShaderResourceView*)source.m_ID.Get())->GetResource(&pSrcResource);
        //D3D11_BOX sourceRegion;
        //sourceRegion.left = src_left;
        //sourceRegion.right = src_right;
        //sourceRegion.top = src_up;
        //sourceRegion.bottom = src_down;
        //sourceRegion.front = 0;
        //sourceRegion.back = 1;
        //RendererBase::Getd3dDeviceContext()->CopySubresourceRegion(pDstResource, D3D11CalcSubresource(mipSlice, arraySlice, m_MipLevels), location_x, location_y, 0, pSrcResource, 0, &sourceRegion);
        //if (m_bHasView)
        //    pDstResource->Release();
        //pSrcResource->Release();
    }
    void Texture2D::CopyInto(Texture2D& source)
    {
        //PT_PROFILE_FUNCTION();
        //ID3D11Resource* pDstResource;
        //ID3D11Resource* pSrcResource;
        //if (m_bHasView)
        //    ((ID3D11ShaderResourceView*)m_ID.Get())->GetResource(&pDstResource);
        //else
        //    pDstResource = (ID3D11Resource*)(m_ID.Get());
        //((ID3D11ShaderResourceView*)source.m_ID.Get())->GetResource(&pSrcResource);
        //RendererBase::Getd3dDeviceContext()->CopyResource(pDstResource, pSrcResource);
        //if (m_bHasView)
        //    pDstResource->Release();
        //pSrcResource->Release();
    }
    void Texture2D::CopyToCPUBuffer(void* buffer)
    {
        //todo rework this function
        //Texture2D cptex;
        //cptex.CreateStack(1280, 720, TextureFormat::RGBA8U, nullptr, (TextureFlags)((unsigned int)TextureFlags::ALLOW_CPU_ACCESS_READ | (unsigned int)TextureFlags::USAGE_STAGING));
        //cptex.CopyInto(*this);
        //D3D11_MAPPED_SUBRESOURCE sr;
        //ZeroMemory(&sr, sizeof(D3D11_MAPPED_SUBRESOURCE));
        //D3D11_TEXTURE2D_DESC desc;
        //ID3D11Texture2D* tex = (ID3D11Texture2D*)cptex.m_ID.Get();
        //tex->GetDesc(&desc);
        //HRESULT hr = Pistachio::RendererBase::Getd3dDeviceContext()->Map(tex, 0, D3D11_MAP_READ, 0, &sr);
        //memcpy(buffer, sr.pData, m_Height * m_Width * RendererUtils::TextureFormatBytesPerPixel(m_format));
        //Pistachio::RendererBase::Getd3dDeviceContext()->Unmap((ID3D11Texture2D*)tex, 0);
    }
    Texture2D* Texture2D::Create(uint32_t width, uint32_t height, RHI::Format format, void* data PT_DEBUG_REGION(, const char* name), TextureFlags flags)
    {
        PT_PROFILE_FUNCTION()
            Texture2D* result = new Texture2D;
        result->m_Width = width;
        result->m_Height = height;
        result->CreateStack(width, height, format, data PT_DEBUG_REGION(, name), flags);
        return result;
    }
    bool Texture2D::operator==(const Texture2D& texture) const
    {
        PT_PROFILE_FUNCTION();
        return (m_ID == texture.m_ID);
    }
}
