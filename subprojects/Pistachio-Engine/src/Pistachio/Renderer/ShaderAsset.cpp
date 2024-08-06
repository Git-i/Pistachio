#include "RootSignature.h"
#include "ptpch.h"
#include "ShaderAsset.h"
#include "Pistachio/Utils/PlatformUtils.h"
namespace Pistachio
{
    std::vector<char>      ShaderAsset::vs;
    ShaderAsset::~ShaderAsset()
    {
        //shader.VS.data = nullptr; //avoid the shader destructor deletin this
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
        RHI::Ptr<RHI::ShaderReflection> PSReflection;
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
        }
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
        desc.VS = {{vs.data()}  ,(uint32_t)vs.size()};
        desc.PS = {{code.data()},(uint32_t)code.size()};
        desc.BlendModes = &blendMode;
        desc.DepthStencilModes = &dsMode;
        desc.RasterizerModes = &rsMode;
        desc.numBlendModes = 1;
        desc.numDepthStencilModes = 1;
        desc.numRasterizerModes = 1;
        desc.NumRenderTargets = 1;
        desc.RTVFormats[0] = RHI::Format::R16G16B16A16_FLOAT;
        desc.shaderMode = RHI::ShaderMode::Memory;


        returnVal->shader.CreateStack(desc, {{1u}}, std::nullopt);
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
