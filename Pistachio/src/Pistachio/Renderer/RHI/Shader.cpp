#include "ptpch.h"
#include "../Shader.h"
#include "../RendererBase.h"
#define VERTEX_SHADER(ID) ((ID3D11VertexShader*)ID.Get())
#define VERTEX_SHADER_PP(ID) ((ID3D11VertexShader**)ID.GetAddressOf())

#define PIXEL_SHADER(ID) ((ID3D11PixelShader*)ID.Get())
#define PIXEL_SHADER_PP(ID) ((ID3D11PixelShader**)ID.GetAddressOf())

#define GEOMETRY_SHADER(ID) ((ID3D11GeometryShader*)ID.Get())
#define GEOMETRY_SHADER_PP(ID) ((ID3D11GeometryShader**)ID.GetAddressOf())

#define BLOB(ID) ((ID3D10Blob*)ID.Get())
#define BLOB_PP(ID) ((ID3D10Blob**)ID.GetAddressOf())

#define INPUT_LAYOUT(ID) ((ID3D11InputLayout*)ID.Get())
#define INPUT_LAYOUT_PP(ID) ((ID3D11InputLayout**)ID.GetAddressOf())

#define BUFFER(ID) ((ID3D11Buffer*)ID.Get())
#define BUFFER_PP(ID) ((ID3D11Buffer**)ID.GetAddressOf())

static RHI::Format RHIFormat(Pistachio::BufferLayoutFormat format)
{
	switch (format)
	{
	case Pistachio::BufferLayoutFormat::FLOAT4: return RHI::Format::R32G32B32A32_FLOAT;
	case Pistachio::BufferLayoutFormat::UINT4: return  RHI::Format::R32G32B32A32_UINT;
	case Pistachio::BufferLayoutFormat::INT4: return   RHI::Format::R32G32B32A32_SINT;
	case Pistachio::BufferLayoutFormat::FLOAT3: return RHI::Format::R32G32B32_FLOAT;
	case Pistachio::BufferLayoutFormat::UINT3: return  RHI::Format::R32G32B32_UINT;
	case Pistachio::BufferLayoutFormat::INT3: return   RHI::Format::R32G32B32_SINT;
	case Pistachio::BufferLayoutFormat::FLOAT2: return RHI::Format::R32G32_FLOAT;
	case Pistachio::BufferLayoutFormat::UINT2: return  RHI::Format::R32G32_UINT;
	case Pistachio::BufferLayoutFormat::INT2: return   RHI::Format::R32G32_SINT;
	case Pistachio::BufferLayoutFormat::FLOAT: return  RHI::Format::R32_FLOAT;
	case Pistachio::BufferLayoutFormat::UINT: return   RHI::Format::R32_UINT;
	case Pistachio::BufferLayoutFormat::INT: return    RHI::Format::R32_SINT;
	default:
		break;
	}
	return RHI::Format::UNKNOWN;
}

namespace Pistachio {
	static RHI::Format ToRHIFormat(BufferLayoutFormat format)
	{
		switch (format)
		{
		case Pistachio::BufferLayoutFormat::FLOAT4: return RHI::Format::R32G32B32A32_FLOAT;
			break;
		case Pistachio::BufferLayoutFormat::UINT4:return RHI::Format::R32G32B32A32_UINT;
			break;
		case Pistachio::BufferLayoutFormat::INT4:return RHI::Format::R32G32B32A32_SINT;
			break;
		case Pistachio::BufferLayoutFormat::FLOAT3:return RHI::Format::R32G32B32_FLOAT;
			break;
		case Pistachio::BufferLayoutFormat::UINT3:return RHI::Format::R32G32B32_UINT;
			break;
		case Pistachio::BufferLayoutFormat::INT3:return RHI::Format::R32G32B32_SINT;
			break;
		case Pistachio::BufferLayoutFormat::FLOAT2:return RHI::Format::R32G32_FLOAT;
			break;
		case Pistachio::BufferLayoutFormat::UINT2:return RHI::Format::R32G32_UINT;
			break;
		case Pistachio::BufferLayoutFormat::INT2:return RHI::Format::R32G32_SINT;
			break;
		case Pistachio::BufferLayoutFormat::FLOAT:return RHI::Format::R32_FLOAT;
			break;
		case Pistachio::BufferLayoutFormat::UINT:return RHI::Format::R32_UINT;
			break;
		case Pistachio::BufferLayoutFormat::INT:return RHI::Format::R32_SINT;
			break;
		default:
			break;
		}
	}
	static void EncodePso(PSOHash& resultBuffer, RHI::PipelineStateObjectDesc* desc)
	{
		//todo if any setting is disabled zero all other settings related to avoid creating different variatons with that setting not
		//even being effective
		memset(&resultBuffer, 0, sizeof(PSOHash));
		resultBuffer.AntialiasedLineEnable = desc->rasterizerMode.AntialiasedLineEnable ? 1 : 0;
		resultBuffer.back_depthFail = (unsigned int)desc->depthStencilMode.BackFace.DepthfailOp;
		resultBuffer.back_fail = (unsigned int)desc->depthStencilMode.BackFace.failOp;
		resultBuffer.back_func = (unsigned int)desc->depthStencilMode.BackFace.Stencilfunc;
		resultBuffer.back_pass = (unsigned int)desc->depthStencilMode.BackFace.passOp;
		resultBuffer.blend_alpha_to_coverage = desc->blendMode.BlendAlphaToCoverage ? 1 : 0;
		resultBuffer.conservative_raster = desc->rasterizerMode.conservativeRaster ? 1 : 0;
		resultBuffer.cullMode = (unsigned int)desc->rasterizerMode.cullMode;
		resultBuffer.depthBias = desc->rasterizerMode.depthBias;
		resultBuffer.depthBiasClamp = desc->rasterizerMode.depthBiasClamp;
		resultBuffer.depthClipEnable = desc->rasterizerMode.depthClipEnable ? 1 : 0;
		resultBuffer.depthEnable = desc->depthStencilMode.DepthEnable;
		resultBuffer.depthFunc = (unsigned int)desc->depthStencilMode.DepthFunc;
		resultBuffer.depthWriteMask = (unsigned int)desc->depthStencilMode.DepthWriteMask;
		resultBuffer.fillMode = (desc->rasterizerMode.fillMode == RHI::FillMode::Solid) ? 1 : 0; //solid is 1, and the tenary operator is unneccesary
		resultBuffer.front_depthFail = (unsigned int)desc->depthStencilMode.FrontFace.DepthfailOp;
		resultBuffer.front_fail = (unsigned int)desc->depthStencilMode.FrontFace.failOp;
		resultBuffer.front_func = (unsigned int)desc->depthStencilMode.FrontFace.Stencilfunc;
		resultBuffer.front_pass = (unsigned int)desc->depthStencilMode.FrontFace.passOp;
		resultBuffer.independent_blend = desc->blendMode.IndependentBlend ? 1 : 0;
		resultBuffer.multi_sample_count = 0;
		if (desc->rasterizerMode.multisampleEnable) resultBuffer.multi_sample_count = (unsigned int)desc->sampleCount;
		resultBuffer.primitiveTopologyType = (unsigned int)desc->rasterizerMode.topology;
		resultBuffer.slopeScaledDepthBias = desc->rasterizerMode.slopeScaledDepthBias;
		int count = desc->numRenderTargets;
		if (desc->blendMode.IndependentBlend) count = 1;
		for (int i = 0; i < count; i++)
		{
			resultBuffer.rtblends[i].enabled = desc->blendMode.blendDescs[i].blendEnable || desc->blendMode.blendDescs[i].logicOpEnable;
			if (!resultBuffer.rtblends[i].enabled) continue;
			PT_CORE_ASSERT(!(desc->blendMode.blendDescs[i].blendEnable && desc->blendMode.blendDescs[i].logicOpEnable)); // invalid
			resultBuffer.rtblends[i].blend_or_logic = desc->blendMode.blendDescs[i].blendEnable ? 1 : 0;
			if (resultBuffer.rtblends[i].blend_or_logic == 0) //logic
			{
				resultBuffer.rtblends[i].srcColorBlendFac_logicOp = (unsigned int)desc->blendMode.blendDescs[i].LogicOp;
			}
			resultBuffer.rtblends[i].alphaOp = (unsigned int)desc->blendMode.blendDescs[i].AlphaBlendOp;
			resultBuffer.rtblends[i].colorBlendOp = (unsigned int)desc->blendMode.blendDescs[i].ColorBlendOp;
			resultBuffer.rtblends[i].dstAlphaBlendFac = (unsigned int)desc->blendMode.blendDescs[i].dstAlphaBlend;
			PT_CORE_ASSERT((unsigned int)desc->blendMode.blendDescs[i].dstAlphaBlend < 8);
			resultBuffer.rtblends[i].dstColorBlendFac = (unsigned int)desc->blendMode.blendDescs[i].dstColorBlend;
			resultBuffer.rtblends[i].srcAlphaBlendFac = (unsigned int)desc->blendMode.blendDescs[i].srcAlphaBlend;
			PT_CORE_ASSERT((unsigned int)desc->blendMode.blendDescs[i].srcAlphaBlend < 8);
			resultBuffer.rtblends[i].srcColorBlendFac_logicOp = (unsigned int)desc->blendMode.blendDescs[i].srcColorBlend;
		}
		resultBuffer.stencilEnable = desc->depthStencilMode.StencilEnable ? 1 : 0;
		resultBuffer.stencilReadMask = desc->depthStencilMode.StencilReadMask;
		resultBuffer.stencilWriteMask = desc->depthStencilMode.StencilWriteMask;
	}
	static PSOHash EncodePso(RHI::PipelineStateObjectDesc* desc)
	{
		PSOHash returnVal;
		EncodePso(returnVal, desc);
		return returnVal;
	}
	Shader::Shader(){}

	Shader* Shader::Create(ShaderCreateDesc* desc)
	{
		Shader* shader = new Shader;
		shader->CreateStack(desc);
		return shader;
	}

	void Shader::Bind()
	{
		RendererBase::GetMainCommandList()->SetRootSignature(rootSig);
		RendererBase::GetMainCommandList()->SetPipelineState(PSOs[currentPSO]);
	}

	void Shader::CreateShaderBinding(ShaderBindingInfo& info)
	{
		for (uint32_t i = 0; i < m_info.sets.size(); i++)
		{
			auto& setinfo = info.sets.emplace_back();
			setinfo = m_info.sets[i];
			setinfo.set = nullptr;
			RendererBase::CreateDescriptorSet(&setinfo.set, layouts[i]);
		}
		
	}

	void Shader::CreateStack(ShaderCreateDesc* desc)
	{
		RHI::PipelineStateObjectDesc PSOdesc;
		//intialize state that doesn't change
		PSOdesc.VS = desc->VS;
		PSOdesc.PS = desc->PS;
		PSOdesc.HS = desc->HS;
		PSOdesc.GS = desc->GS;
		PSOdesc.DS = desc->DS;
		if(desc->VS) VS = desc->VS;
		if(desc->PS) PS = desc->PS;
		if(desc->HS) HS = desc->HS;
		if(desc->GS) GS = desc->GS;
		if(desc->DS) DS = desc->DS;
		//only suppport 6 rtvs
		PSOdesc.RTVFormats[0] = desc->RTVFormats[0];
		PSOdesc.RTVFormats[1] = desc->RTVFormats[1];
		PSOdesc.RTVFormats[2] = desc->RTVFormats[2];
		PSOdesc.RTVFormats[3] = desc->RTVFormats[3];
		PSOdesc.RTVFormats[4] = desc->RTVFormats[4];
		PSOdesc.RTVFormats[5] = desc->RTVFormats[5];
		rtvFormats[0] = desc->RTVFormats[0];
		rtvFormats[1] = desc->RTVFormats[1];
		rtvFormats[2] = desc->RTVFormats[2];
		rtvFormats[3] = desc->RTVFormats[3];
		rtvFormats[4] = desc->RTVFormats[4];
		rtvFormats[5] = desc->RTVFormats[5];
		PSOdesc.numRenderTargets = desc->NumRenderTargets;
		numRenderTargets = desc->NumRenderTargets;
		PSOdesc.DSVFormat = desc->DSVFormat;
		dsvFormat = desc->DSVFormat;
		CreateRootSignature();
		PSOdesc.rootSig = rootSig;
		//make this dynamic enough to accomodate instancing
		InputBindingDescription = new RHI::InputBindingDesc;
		InputBindingDescription->inputRate = RHI::InputRate::Vertex;
		InputBindingDescription->stride = desc->InputDescription[desc->numInputs - 1].Offset + BufferLayoutFormatSize(desc->InputDescription[desc->numInputs - 1].Format);
		InputElementDescription = new RHI::InputElementDesc[desc->numInputs];
		numInputBindings = 1;
		numInputElements = desc->numInputs;
		for (uint32_t i = 0; i < desc->numInputs; i++)
		{
			InputElementDescription[i].alignedByteOffset = desc->InputDescription[i].Offset; 
			InputElementDescription[i].format = ToRHIFormat(desc->InputDescription[i].Format); 
			InputElementDescription[i].inputSlot = 0;
			InputElementDescription[i].location = i;
		}
		PSOdesc.inputBindings = InputBindingDescription;
		PSOdesc.numInputBindings = 1;
		PSOdesc.inputElements = InputElementDescription;
		PSOdesc.numInputElements = desc->numInputs;
		for (uint32_t depthMode = 0; depthMode < desc->numDepthStencilModes; depthMode++)
		{
			PSOdesc.depthStencilMode = desc->DepthStencilModes[depthMode];
			for (uint32_t rasterizerMode = 0; rasterizerMode < desc->numRasterizerModes; rasterizerMode++)
			{
				PSOdesc.rasterizerMode = desc->RasterizerModes[rasterizerMode];
				for (uint32_t blendMode = 0; blendMode < desc->numBlendModes; blendMode++)
				{
					PSOdesc.blendMode = desc->BlendModes[blendMode];
					RHI::PipelineStateObject* pso;
					RendererBase::Getd3dDevice()->CreatePipelineStateObject(&PSOdesc, &pso);
					PSOs[EncodePso(&PSOdesc)] = pso;
				}
			}
		}
		currentPSO = PSOs.begin()->first;
	}

	void Shader::CreateRootSignature()
	{
		// Creating an RHI::RootSignature
		// we first need to reflect the vertex and pixel shaders
		RHI::ShaderReflection* VSreflection;
		RHI::ShaderReflection* PSreflection;
		RHI::RootSignatureDesc RSdesc;
		std::vector<RHI::RootParameterDesc> rootParams;
		std::vector<RHI::DescriptorRange> ranges;
		uint32_t rangeOffset = 0;
		if (!VS.empty())
		{
			RHI::ShaderReflection::CreateFromFile(VS.c_str(), &VSreflection);
			uint32_t numSets = VSreflection->GetNumDescriptorSets();
			std::vector<RHI::SRDescriptorSet> sets(numSets);
			VSreflection->GetAllDescriptorSets(sets.data());
			for (uint32_t i = 0; i < numSets; i++)
			{
				auto& setInfo= m_info.sets.emplace_back();
				setInfo.setIndex = sets[i].setIndex;
				RHI::RootParameterDesc desc;

				desc.type = RHI::RootParameterType::DescriptorTable;
				desc.descriptorTable.numDescriptorRanges = sets[i].bindingCount;
				desc.descriptorTable.ranges = (RHI::DescriptorRange*)rangeOffset;

				std::vector<RHI::SRDescriptorBinding> bindings(sets[i].bindingCount);
				VSreflection->GetDescriptorSetBindings(&sets[i], bindings.data());

				//fill out all the ranges (descriptors)
				for (uint32_t j = 0; j < sets[i].bindingCount; j++)
				{
					RHI::DescriptorRange range;

					setInfo.slot.push_back(bindings[j].bindingSlot);
					setInfo.type.push_back(bindings[j].resourceType);
					setInfo.stage.push_back(RHI::ShaderStage::Vertex);
					setInfo.count.push_back(bindings[j].count);

					range.BaseShaderRegister = bindings[j].bindingSlot;
					range.numDescriptors = bindings[j].count;
					range.type = bindings[j].resourceType;
					range.stage = RHI::ShaderStage::Vertex;
					ranges.push_back(range);
					rangeOffset++;
				}
				rootParams.push_back(desc);
			}
		}
		if (!PS.empty())
		{
			RHI::ShaderReflection::CreateFromFile(PS.c_str(), &PSreflection);
			uint32_t numSets = PSreflection->GetNumDescriptorSets();
			std::vector<RHI::SRDescriptorSet> sets(numSets);
			PSreflection->GetAllDescriptorSets(sets.data());
			for (uint32_t i = 0; i < numSets; i++)
			{

				auto& setInfo = m_info.sets.emplace_back();
				setInfo.setIndex = sets[i].setIndex;
				RHI::RootParameterDesc desc;

				desc.type = RHI::RootParameterType::DescriptorTable;
				desc.descriptorTable.numDescriptorRanges = sets[i].bindingCount;
				desc.descriptorTable.ranges = (RHI::DescriptorRange*)rangeOffset;

				std::vector<RHI::SRDescriptorBinding> bindings(sets[i].bindingCount);
				PSreflection->GetDescriptorSetBindings(&sets[i], bindings.data());

				//fill out all the ranges (descriptors)
				for (uint32_t j = 0; j < sets[i].bindingCount; j++)
				{
					setInfo.slot.push_back(bindings[j].bindingSlot);
					setInfo.type.push_back(bindings[j].resourceType);
					setInfo.stage.push_back(RHI::ShaderStage::Pixel);
					setInfo.count.push_back(bindings[i].count);

					RHI::DescriptorRange range;
					range.BaseShaderRegister = bindings[j].bindingSlot;
					range.numDescriptors = bindings[j].count;
					range.type = bindings[j].resourceType;
					range.stage = RHI::ShaderStage::Pixel;
					ranges.emplace_back(range);
					rangeOffset++;
				}
				rootParams.push_back(desc);
			}
		}
		for (auto& param : rootParams)
		{
			param.descriptorTable.ranges = &ranges[(size_t)param.descriptorTable.ranges];
		}
		RSdesc.numRootParameters = rootParams.size();
		RSdesc.rootParameters = rootParams.data();
		numLayouts = rootParams.size();
		layouts = new RHI::DescriptorSetLayout*[numLayouts];
		RendererBase::Getd3dDevice()->CreateRootSignature(&RSdesc, &rootSig, layouts);
	}



	void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& Shader)
	{
		PT_PROFILE_FUNCTION();
		PT_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end());
		m_Shaders[name] = Shader;
	}
	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& vertex, const std::string& fragment)
	{
		PT_PROFILE_FUNCTION();
		auto shader = std::make_shared<Shader>();// ((wchar_t*)(vertex.c_str()), (wchar_t*)(fragment.c_str()));
		Add(name, shader);
		return shader;
	}
	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		PT_PROFILE_FUNCTION();
		PT_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end());
		return m_Shaders[name];
	}



	void ShaderBindingInfo::UpdateBufferBinding(uint32_t setInfosIndex, BufferBindingUpdateDesc* desc, uint32_t slot)
	{
		RHI::DescriptorBufferInfo info;
		info.buffer = desc->buffer;
		info.offset = desc->offset;
		info.range = desc->size;
		RHI::DescriptorSetUpdateDesc updateDesc;
		updateDesc.arrayIndex = 0;
		updateDesc.binding = slot;
		updateDesc.numDescriptors = 1;
		updateDesc.type = RHI::DescriptorType::CBV;
		updateDesc.bufferInfos = &info;
		RendererBase::Getd3dDevice()->UpdateDescriptorSets(1, &updateDesc, sets[setInfosIndex].set);
	}

}

