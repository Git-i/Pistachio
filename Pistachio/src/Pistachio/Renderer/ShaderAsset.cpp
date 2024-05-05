#include "ptpch.h"
#include "ShaderAsset.h"
#include "Pistachio\Utils\PlatformUtils.h"
namespace Pistachio
{
    std::vector<char>      ShaderAsset::vs;
    RHI::ShaderReflection* ShaderAsset::VSReflection;
    ShaderAsset::~ShaderAsset()
    {
        shader.VS.data = nullptr; //avoid the shader destructor deletin this
    }
    ShaderAsset* ShaderAsset::Create(const char* filename)
    {
        ShaderAsset* returnVal = new ShaderAsset;
        std::ifstream infile(filename, std::ios::binary);
        PT_CORE_ASSERT(infile);
        uint32_t numParams = 0;
        infile.read((char*)&numParams, sizeof(uint32_t));
        numParams = Edian::ConvertToSystemEndian(numParams, Pistachio::Big);
        uint32_t furthestOffset = 0;
        uint32_t furthestElementSize = 0;
        for (uint32_t i = 0; i < numParams; i++)
        {
            std::string paramName;
            uint32_t strSize = 0;
            infile.read((char*)&strSize, sizeof(uint32_t));
            strSize = Edian::ConvertToSystemEndian(strSize, Pistachio::Big);
            paramName.resize(strSize + 1);
            infile.read((char*)paramName.data(), strSize);
            paramName += '\0';

            uint32_t offset = 0;
            infile.read((char*)&offset, sizeof(uint32_t));
            offset = Edian::ConvertToSystemEndian(offset, Pistachio::Big);
            uint32_t type = 0;
            infile.read((char*)&type, sizeof(uint32_t));
            type = Edian::ConvertToSystemEndian(type, Pistachio::Big);
            if (offset > furthestOffset)
            {
                furthestOffset = offset;
                furthestElementSize = ((type % 4) + 1) * 4;
            }
            returnVal->parametersMap[std::move(paramName)] =
            {
                offset,
                (ParamType)type
            };
        }
        returnVal->paramBufferSize = furthestOffset + furthestElementSize;
        uint32_t numBindings = 0;
        infile.read((char*)&numBindings, sizeof(uint32_t));
        numBindings = Edian::ConvertToSystemEndian(numBindings, Pistachio::Big);
        for (uint32_t i = 0; i < numBindings; i++)
        {
            std::string bindingName;
            uint32_t strSize = 0;
            infile.read((char*)&strSize, sizeof(uint32_t));
            strSize = Edian::ConvertToSystemEndian(strSize, Pistachio::Big);
            bindingName.resize(strSize + 1);
            infile.read((char*)bindingName.data(), strSize);
            bindingName += '\0';

            uint32_t slot = 0;
            infile.read((char*)&slot, sizeof(uint32_t));
            slot = Edian::ConvertToSystemEndian(slot, Pistachio::Big);
            returnVal->bindingsMap[std::move(bindingName)] = slot;
        }
        RHI::API api = RendererBase::instance->GetInstanceAPI();
        uint32_t spvSize = 0;
        infile.read((char*)&spvSize, sizeof(uint32_t));
        spvSize = Edian::ConvertToSystemEndian(spvSize, Pistachio::Big);
        std::vector<char> code;
        RHI::ShaderReflection* PSReflection = nullptr;
        if (api == RHI::API::Vulkan)
        {
            code.resize(spvSize);
            infile.read(code.data(), spvSize);
        }
        else //(api == RHI::API::DX12)
        {
            infile.seekg(spvSize, std::ios::cur);
            uint32_t dxilSize = 0;
            infile.read((char*)&dxilSize, sizeof(uint32_t));
            dxilSize = Edian::ConvertToSystemEndian(dxilSize, Pistachio::Big);
            code.resize(dxilSize);
            infile.read(code.data(), dxilSize);
        }
        
        if (vs.size() <= 0)
        {
            std::ifstream vertexShaderFile("resources/shaders/vertex/Compiled/VertexShader", std::ios::binary | std::ios::ate);
            size_t vsSize = vertexShaderFile.tellg();
            vs.resize(vsSize);
            vertexShaderFile.seekg(0, std::ios::beg);
            vertexShaderFile.read(vs.data(), vsSize);
            RHI::ShaderReflection::CreateFromMemory(vs.data(), (uint32_t)vs.size(), &VSReflection);
        }
        RHI::ShaderReflection::CreateFromMemory(code.data(), (uint32_t)code.size(), &PSReflection);
        RHI::BlendMode blendMode{};
        blendMode.BlendAlphaToCoverage = false;
        blendMode.IndependentBlend = true;
        blendMode.blendDescs[0].blendEnable = false;
        RHI::DepthStencilMode dsMode{};
        dsMode.DepthEnable = false;
        dsMode.StencilEnable = false;
        RHI::RasterizerMode rsMode{};
        rsMode.cullMode = RHI::CullMode::None;
        rsMode.fillMode = RHI::FillMode::Solid;
        rsMode.topology = RHI::PrimitiveTopology::TriangleList;
        ShaderCreateDesc desc{};
        desc.VS = {vs.data()  ,(uint32_t)vs.size()};
        desc.PS = {code.data(),(uint32_t)code.size()};
        desc.BlendModes = &blendMode;
        desc.DepthStencilModes = &dsMode;
        desc.RasterizerModes = &rsMode;
        desc.numBlendModes = 1;
        desc.numDepthStencilModes = 1;
        desc.numRasterizerModes = 1;
        desc.NumRenderTargets = 1;
        desc.RTVFormats[0] = RHI::Format::R16G16B16A16_FLOAT;
        desc.shaderMode = RHI::ShaderMode::Memory;
        returnVal->shader.VS = { vs.data(),(uint32_t)vs.size()};
        returnVal->shader.PS = { (char*)malloc(code.size()), (uint32_t)code.size()};
        memcpy(returnVal->shader.PS.data, code.data(), code.size());
        returnVal->shader.CreateSetInfos(VSReflection, PSReflection);
        PSReflection->Release();

        RHI::RootParameterDesc rpDesc[5];
        rpDesc[0].type = RHI::RootParameterType::DescriptorTable;
        rpDesc[0].descriptorTable.numDescriptorRanges = 1;
        rpDesc[0].descriptorTable.setIndex = 0;
        RHI::DescriptorRange frameCB;
        frameCB.BaseShaderRegister = 0;
        frameCB.numDescriptors = 1;
        frameCB.stage = RHI::ShaderStage::Vertex;
        frameCB.type = RHI::DescriptorType::ConstantBuffer;
        rpDesc[0].descriptorTable.ranges = &frameCB;

        rpDesc[1].type = RHI::RootParameterType::DynamicDescriptor;
        rpDesc[1].dynamicDescriptor.setIndex = 1;
        rpDesc[1].dynamicDescriptor.stage = RHI::ShaderStage::Vertex;
        rpDesc[1].dynamicDescriptor.type = RHI::DescriptorType::ConstantBufferDynamic;

        rpDesc[2].type = RHI::RootParameterType::DescriptorTable;
        rpDesc[2].descriptorTable.setIndex = 2;
        rpDesc[2].descriptorTable.numDescriptorRanges = 4;//BRDF, irradiance, prefilter, shadowMap
        RHI::DescriptorRange rendererRanges[4];
        rendererRanges[0].numDescriptors = 1;
        rendererRanges[0].stage = RHI::ShaderStage::Pixel;
        rendererRanges[0].type = RHI::DescriptorType::SampledTexture;
        rendererRanges[1] = rendererRanges[2] = rendererRanges[3] = rendererRanges[0];
        for (uint32_t i = 0; i < 4; i++) rendererRanges[i].BaseShaderRegister = i;
        rpDesc[2].descriptorTable.ranges = rendererRanges;

        rpDesc[3].type = RHI::RootParameterType::DescriptorTable;//Parameters constant buffer
        //user data
        std::vector<RHI::DescriptorRange> customRange;
        customRange.reserve(returnVal->bindingsMap.size());
        for (auto& [_, index] : returnVal->bindingsMap)
        {
            auto& range = customRange.emplace_back();
            range.BaseShaderRegister = index;
            range.numDescriptors = 1;
            range.stage = RHI::ShaderStage::Pixel;
            range.type = RHI::DescriptorType::SampledTexture;
        }
        rpDesc[3].descriptorTable.numDescriptorRanges = (uint32_t)customRange.size();
        rpDesc[3].descriptorTable.ranges = customRange.data();
        rpDesc[3].descriptorTable.setIndex = 3;

        rpDesc[4].type = RHI::RootParameterType::DynamicDescriptor;
        rpDesc[4].dynamicDescriptor.setIndex = 4;
        rpDesc[4].dynamicDescriptor.type = RHI::DescriptorType::ConstantBufferDynamic;
        rpDesc[4].dynamicDescriptor.stage = RHI::ShaderStage::Pixel;

        RHI::RootSignatureDesc rsDesc;
        rsDesc.numRootParameters = 5;
        rsDesc.rootParameters = rpDesc;
        
        RHI::RootSignature* rs;
        RHI::DescriptorSetLayout* layouts[5];
        RendererBase::device->CreateRootSignature(&rsDesc, &rs, layouts);

        returnVal->shader.Initialize(&desc, rs);
        returnVal->shader.layouts = new RHI::DescriptorSetLayout * [4];
        for (uint32_t i = 0; i < 4; i++)
        {
            returnVal->shader.layouts[i] = layouts[i];
        }
        rs->Release();
        
        return returnVal;
    }
    ParamInfo ShaderAsset::GetParameterInfo(const std::string& paramName)
    {
        if (parametersMap.find(paramName) != parametersMap.end())
        {
            return parametersMap.at(paramName);
        }
        PT_CORE_ERROR("Invalid Shader Parameter Name");
        return ParamInfo{ UINT32_MAX, (ParamType)UINT32_MAX };
    }
}
