#include "ptpch.h"
#include "Shader.h"
#include "RendererBase.h"
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
		default: return RHI::Format::UNKNOWN;
			break;
		}
	}
	static void EncodePso(PSOHash& resultBuffer, RHI::PipelineStateObjectDesc* desc)
	{
		//we dont zero state if its disabled so the user can safely enable it with the past config
		//we instead zero disabled state if we want to compare two PSOHash structures
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
		//encode all 6 even if independent blend
		for (uint32_t i = 0; i < desc->numRenderTargets; i++)
		{
			resultBuffer.rtblends[i].enabled = desc->blendMode.blendDescs[i].blendEnable || desc->blendMode.blendDescs[i].logicOpEnable;
			if (!resultBuffer.rtblends[i].enabled) continue;
			if (!desc->blendMode.IndependentBlend && i > 0)
			{
				PT_CORE_ASSERT(!(desc->blendMode.blendDescs[i].blendEnable && desc->blendMode.blendDescs[i].logicOpEnable)); // invalid
				PT_CORE_ASSERT((unsigned int)desc->blendMode.blendDescs[i].srcAlphaBlend < 8);
				PT_CORE_ASSERT((unsigned int)desc->blendMode.blendDescs[i].dstAlphaBlend < 8);
			}
			resultBuffer.rtblends[i].blend_or_logic = desc->blendMode.blendDescs[i].blendEnable ? 1 : 0;
			if (resultBuffer.rtblends[i].blend_or_logic == 0) //logic
			{
				resultBuffer.rtblends[i].srcColorBlendFac_logicOp = (unsigned int)desc->blendMode.blendDescs[i].logicOp;
			}
			else
			{
				resultBuffer.rtblends[i].srcColorBlendFac_logicOp = (unsigned int)desc->blendMode.blendDescs[i].srcColorBlend;//blend
			}
			resultBuffer.rtblends[i].alphaOp = (unsigned int)desc->blendMode.blendDescs[i].AlphaBlendOp;
			resultBuffer.rtblends[i].colorBlendOp = (unsigned int)desc->blendMode.blendDescs[i].ColorBlendOp;
			resultBuffer.rtblends[i].dstAlphaBlendFac = (unsigned int)desc->blendMode.blendDescs[i].dstAlphaBlend;
			resultBuffer.rtblends[i].dstColorBlendFac = (unsigned int)desc->blendMode.blendDescs[i].dstColorBlend;
			resultBuffer.rtblends[i].srcAlphaBlendFac = (unsigned int)desc->blendMode.blendDescs[i].srcAlphaBlend;
		}
		resultBuffer.stencilEnable = desc->depthStencilMode.StencilEnable ? 1 : 0;
		resultBuffer.stencilReadMask = desc->depthStencilMode.StencilReadMask;
		resultBuffer.stencilWriteMask = desc->depthStencilMode.StencilWriteMask;
	}
	PSOHash PSOComparsionState(const PSOHash* pso)
	{
		PSOHash returnVal = {};
		memcpy(&returnVal, pso, sizeof(PSOHash));
		//invalidate depth state if depth test is off
		//is depth bias affected??
		if (returnVal.depthEnable == false)
		{
			returnVal.depthFunc = 0;
			returnVal.depthWriteMask = 0;
		}
		if (returnVal.stencilEnable == false)
		{
			returnVal.stencilWriteMask = 0;
			returnVal.stencilReadMask = 0;

			returnVal.back_depthFail = 0;
			returnVal.back_fail = 0;
			returnVal.back_pass = 0;
			returnVal.back_func = 0;

			returnVal.front_depthFail = 0;
			returnVal.front_fail = 0;
			returnVal.front_pass = 0;
			returnVal.front_func = 0;
		}
		//invalidate all other 5/6 targets
		if (returnVal.independent_blend == true)
		{
			if (returnVal.rtblends[0].enabled == 0)
			{
				memset(returnVal.rtblends, 0, sizeof(PSOHash::RTBlend) * 6);
				returnVal.blend_alpha_to_coverage = 0;
			}
			else
			{
				memset(&returnVal.rtblends[1], 0, sizeof(PSOHash::RTBlend) * 5);
				//blend or logic is enabled
				if (returnVal.rtblends->blend_or_logic == 0) //logic Op
				{
					//invalidate non logic settings
					returnVal.rtblends->colorBlendOp = 0;
					returnVal.rtblends->enabled = 0;
					returnVal.rtblends->dstColorBlendFac = 0;
					returnVal.rtblends->srcAlphaBlendFac = 0;
					returnVal.rtblends->dstAlphaBlendFac = 0;
					returnVal.rtblends->alphaOp = 0;
				}
			}
		}
		else
		{
			for (uint32_t i = 0; i < 6; i++)
			{
				//invalidate the entire struct and continue
				if (returnVal.rtblends[i].enabled == 0)
				{
					memset(returnVal.rtblends, 0, sizeof(PSOHash::RTBlend));
					returnVal.blend_alpha_to_coverage = 0;
					
					continue;
				}
				//blend or logic is enabled
				if (returnVal.rtblends->blend_or_logic == 0) //logic Op
				{
					//invalidate non logic settings
					returnVal.blend_alpha_to_coverage = 0;
					returnVal.rtblends[i].colorBlendOp = 0;
					returnVal.rtblends[i].enabled = 0;
					returnVal.rtblends[i].dstColorBlendFac = 0;
					returnVal.rtblends[i].srcAlphaBlendFac = 0;
					returnVal.rtblends[i].dstAlphaBlendFac = 0;
					returnVal.rtblends[i].alphaOp = 0;
				}
			}
		}
		return returnVal;
	}
	static PSOHash EncodePso(RHI::PipelineStateObjectDesc* desc)
	{
		PSOHash returnVal;
		EncodePso(returnVal, desc);
		return returnVal;
	}
	static void DecodePso(RHI::PipelineStateObjectDesc* outbuf, PSOHash* hash)
	{
		outbuf->blendMode.BlendAlphaToCoverage = hash->blend_alpha_to_coverage;
		outbuf->blendMode.IndependentBlend = hash->independent_blend;
		for (uint32_t i = 0; i < 6; i++)
		{
			outbuf->blendMode.blendDescs[i].AlphaBlendOp  = (RHI::BlendOp)hash->rtblends[i].alphaOp;
			outbuf->blendMode.blendDescs[i].blendEnable   = hash->rtblends[i].blend_or_logic && hash->rtblends[i].enabled;
			outbuf->blendMode.blendDescs[i].ColorBlendOp  = (RHI::BlendOp)hash->rtblends[i].colorBlendOp;
			outbuf->blendMode.blendDescs[i].dstAlphaBlend = (RHI::BlendFac)hash->rtblends[i].dstAlphaBlendFac;
			outbuf->blendMode.blendDescs[i].dstColorBlend = (RHI::BlendFac)hash->rtblends[i].dstColorBlendFac;
			outbuf->blendMode.blendDescs[i].logicOp       = (RHI::LogicOp)hash->rtblends[i].srcColorBlendFac_logicOp;
			outbuf->blendMode.blendDescs[i].logicOpEnable = hash->rtblends[i].blend_or_logic && hash->rtblends[i].enabled;
			outbuf->blendMode.blendDescs[i].srcAlphaBlend = (RHI::BlendFac)hash->rtblends[i].srcAlphaBlendFac;
			outbuf->blendMode.blendDescs[i].srcColorBlend = (RHI::BlendFac)hash->rtblends[i].srcColorBlendFac_logicOp;
			outbuf->blendMode.blendDescs[i].writeMask = 1;//todo handle write masks
		}
		outbuf->depthStencilMode.BackFace.DepthfailOp = (RHI::StencilOp)hash->back_depthFail;
		outbuf->depthStencilMode.BackFace.failOp = (RHI::StencilOp)hash->back_fail;
		outbuf->depthStencilMode.BackFace.passOp = (RHI::StencilOp)hash->back_pass;
		outbuf->depthStencilMode.BackFace.Stencilfunc = (RHI::ComparisonFunc)hash->back_func;
		outbuf->depthStencilMode.FrontFace.DepthfailOp = (RHI::StencilOp)hash->front_depthFail;
		outbuf->depthStencilMode.FrontFace.failOp = (RHI::StencilOp)hash->front_fail;
		outbuf->depthStencilMode.FrontFace.passOp = (RHI::StencilOp)hash->front_pass;
		outbuf->depthStencilMode.FrontFace.Stencilfunc = (RHI::ComparisonFunc)hash->front_func;
		outbuf->depthStencilMode.DepthEnable = hash->depthEnable;
		outbuf->depthStencilMode.DepthFunc = (RHI::ComparisonFunc)hash->depthFunc;
		outbuf->depthStencilMode.DepthWriteMask = (RHI::DepthWriteMask)hash->depthWriteMask;
		outbuf->depthStencilMode.StencilEnable = hash->stencilEnable;
		outbuf->depthStencilMode.StencilReadMask = hash->stencilReadMask;
		outbuf->depthStencilMode.StencilWriteMask = hash->stencilWriteMask;
		outbuf->rasterizerMode.AntialiasedLineEnable = hash->AntialiasedLineEnable;
		outbuf->rasterizerMode.conservativeRaster = hash->conservative_raster;
		outbuf->rasterizerMode.cullMode = (RHI::CullMode)hash->cullMode;
		outbuf->rasterizerMode.depthBias = hash->depthBias;
		outbuf->rasterizerMode.depthBiasClamp = hash->depthBiasClamp;
		outbuf->rasterizerMode.depthClipEnable = hash->depthClipEnable;
		outbuf->rasterizerMode.fillMode = (RHI::FillMode)hash->fillMode;
		outbuf->rasterizerMode.multisampleEnable = hash->multi_sample_count == 0;
		outbuf->rasterizerMode.slopeScaledDepthBias = hash->slopeScaledDepthBias;
		outbuf->rasterizerMode.topology = (RHI::PrimitiveTopology)hash->primitiveTopologyType;
		outbuf->sampleCount = (RHI::SampleMode)hash->multi_sample_count;
	}
	Shader::Shader(){}

	Shader::~Shader()
	{
		if (VS.data) free(VS.data);
		if (PS.data) free(PS.data);
		if (HS.data) free(HS.data);
		if (GS.data) free(GS.data);
		if (DS.data) free(DS.data);
		for (uint32_t i = 0; i < numLayouts; i++)
		{
			layouts[i]->Release();
		}
		rootSig->Release();
		delete[] layouts;
	}

	Shader* Shader::Create(ShaderCreateDesc* desc)
	{
		Shader* shader = new Shader;
		shader->CreateStack(desc);
		return shader;
	}

	Shader* Shader::CreateWithRs(ShaderCreateDesc* desc, RHI::RootSignature* rs,RHI::DescriptorSetLayout** layouts, uint32_t numLayouts)
	{
		Shader* shader = new Shader;
		shader->CreateStackRs(desc, rs, layouts, numLayouts);
		return shader;
	}
	void Shader::CreateStackRs(ShaderCreateDesc* desc, RHI::RootSignature* rs, RHI::DescriptorSetLayout** in_layouts, uint32_t in_numLayouts)
	{
		RHI::ShaderReflection* VSReflection = nullptr;
		RHI::ShaderReflection* PSReflection = nullptr;
		if (desc->VS.data)
		{
			VS = desc->VS;
			if (desc->shaderMode == RHI::ShaderMode::File) RHI::ShaderReflection::CreateFromFile(desc->VS.data, &VSReflection);
			else RHI::ShaderReflection::CreateFromMemory(desc->VS.data, desc->VS.size, &VSReflection);
		}
		if (desc->PS.data)
		{
			PS = desc->PS;
			if (desc->shaderMode == RHI::ShaderMode::File) RHI::ShaderReflection::CreateFromFile(desc->PS.data, &PSReflection);
			else RHI::ShaderReflection::CreateFromMemory(desc->PS.data, desc->PS.size, &PSReflection);
		}
		HS = desc->HS;
		GS = desc->GS;
		DS = desc->DS;
		CreateSetInfos(VSReflection, PSReflection);
		if(VSReflection) VSReflection->Release();
		if(PSReflection) PSReflection->Release();
		Initialize(desc, rs);
		numLayouts = in_numLayouts;
		layouts = new RHI::DescriptorSetLayout * [numLayouts];
		for (uint32_t i = 0; i < in_numLayouts; i++)
		{
			layouts[i] = in_layouts[i];
			if (layouts[i]) layouts[i]->Hold();
		}
	}

	void Shader::Bind(RHI::GraphicsCommandList* list)
	{
		list->SetRootSignature(rootSig);
		list->SetPipelineState(PSOs[currentPSO]);
	}

	void Shader::GetDepthStencilMode(RHI::DepthStencilMode* mode)
	{
		mode->BackFace.DepthfailOp = (RHI::StencilOp)currentPSO.back_depthFail;
		mode->BackFace.failOp	   = (RHI::StencilOp)currentPSO.back_fail;
		mode->BackFace.passOp	   = (RHI::StencilOp)currentPSO.back_pass;
		mode->BackFace.Stencilfunc = (RHI::ComparisonFunc)currentPSO.back_func;
		mode->FrontFace.DepthfailOp = (RHI::StencilOp)currentPSO.front_depthFail;
		mode->FrontFace.failOp	    = (RHI::StencilOp)currentPSO.front_fail;
		mode->FrontFace.passOp	    = (RHI::StencilOp)currentPSO.front_pass;
		mode->FrontFace.Stencilfunc = (RHI::ComparisonFunc)currentPSO.front_func;
		mode->DepthEnable = currentPSO.depthEnable;
		mode->DepthFunc = (RHI::ComparisonFunc)currentPSO.depthFunc;
		mode->DepthWriteMask = (RHI::DepthWriteMask)currentPSO.depthWriteMask;
		mode->StencilEnable = currentPSO.stencilEnable;
		mode->StencilReadMask = currentPSO.stencilReadMask;
		mode->StencilWriteMask = currentPSO.stencilWriteMask;
	}

	uint32_t Shader::SetDepthStencilMode(RHI::DepthStencilMode* mode, ShaderModeSetFlags flags)
	{
		PSOHash newHash = currentPSO;
		newHash.back_depthFail      = (std::uint32_t)mode->BackFace.DepthfailOp  ;
		newHash.back_fail           = (std::uint32_t)mode->BackFace.failOp       ;
		newHash.back_pass           = (std::uint32_t)mode->BackFace.passOp       ;
		newHash.back_func           = (std::uint32_t)mode->BackFace.Stencilfunc  ;
		newHash.front_depthFail     = (std::uint32_t)mode->FrontFace.DepthfailOp ;
		newHash.front_fail          = (std::uint32_t)mode->FrontFace.failOp      ;
		newHash.front_pass          = (std::uint32_t)mode->FrontFace.passOp      ;
		newHash.front_func          = (std::uint32_t)mode->FrontFace.Stencilfunc ;
		newHash.depthEnable         =                mode->DepthEnable           ;
		newHash.depthFunc           = (std::uint32_t)mode->DepthFunc             ;
		newHash.depthWriteMask      = (std::uint32_t)mode->DepthWriteMask        ;
		newHash.stencilEnable       =                mode->StencilEnable         ;
		newHash.stencilReadMask     =                mode->StencilReadMask       ;
		newHash.stencilWriteMask    =                mode->StencilWriteMask      ;
		if (auto search = PSOs.find(newHash); search != PSOs.end())
		{
			//pso exists
			PT_CORE_INFO("Matching PSO found, setting DS mode");
			currentPSO = newHash;
			// todo bind the new PSO ? what is this is not the bound shader?
			return 0;
		}
		else
		{
			if (flags == ShaderModeSetFlags::AutomaticallyCreate)
			{
				PT_CORE_WARN("Creating new PSO to match current DS mode");
				RHI::PipelineStateObjectDesc desc{};
				desc.rootSig = rootSig;
				DecodePso(&desc, &newHash);
				RHI::PipelineStateObject* pso;
#if _DEBUG
				auto h1 = PSOComparsionState(&newHash);
				if (XXH64(&h1, sizeof(Pistachio::PSOHash), 10) != XXH64(&newHash, sizeof(Pistachio::PSOHash), 10))
				{
					PT_CORE_WARN("{0} A disabled state still had valid configuration, this config would be zeroed during comparisons", __FUNCTION__);
				}
#endif
				desc.VS = VS;
				desc.PS = PS;
				desc.GS = GS;
				desc.HS = HS;
				desc.DS = DS;
				desc.DSVFormat = dsvFormat;
				desc.inputBindings = InputBindingDescription;
				desc.inputElements = InputElementDescription;
				desc.numInputBindings = numInputBindings;
				desc.numInputElements = numInputElements;
				desc.numRenderTargets = numRenderTargets;
				memcpy(desc.RTVFormats, rtvFormats, sizeof(RHI::Format) * 6);
				RendererBase::Getd3dDevice()->CreatePipelineStateObject(&desc, &pso);
				currentPSO = newHash;
				PSOs[newHash] = pso;
				//set the pso
				return 0;
			}
			return  1;
		}
	}



	void Shader::GetVSShaderBinding(SetInfo& info, uint32_t setIndex)
	{
		uint32_t index = UINT32_MAX;
		for (uint32_t i = 0; i < m_VSinfo.sets.size(); i++)
		{
			if (m_VSinfo.sets[i].setIndex == setIndex) index = i;
		}
		info = m_VSinfo.sets[index];
		RendererBase::device->CreateDescriptorSets(RendererBase::heap, 1, layouts[index], &info.set);
	}

	void Shader::GetPSShaderBinding(SetInfo& info, uint32_t setIndex)
	{
		//we make the assumption that vertex shaders come first in the layout array
		uint32_t index = UINT32_MAX;
		for (uint32_t i = 0; i < m_PSinfo.sets.size(); i++)
		{
			if (m_PSinfo.sets[i].setIndex == setIndex) index = i;
		}
		info = m_PSinfo.sets[index];
		RendererBase::device->CreateDescriptorSets(RendererBase::heap, 1, layouts[setIndex], &info.set);
	}

	void Shader::ApplyBinding(RHI::GraphicsCommandList* list,const SetInfo& info)
	{
		list->BindDescriptorSet(rootSig, info.set, info.setIndex);
	}

	
	void Shader::Initialize(ShaderCreateDesc* desc, RHI::RootSignature* signature)
	{
		RHI::PipelineStateObjectDesc PSOdesc;
		//intialize state that doesn't change
		PSOdesc.VS = desc->VS;
		PSOdesc.PS = desc->PS;
		PSOdesc.HS = desc->HS;
		PSOdesc.GS = desc->GS;
		PSOdesc.DS = desc->DS;
		PSOdesc.shaderMode = desc->shaderMode;
		mode = desc->shaderMode;
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
		PSOdesc.rootSig = signature;
		signature->Hold();//ensure that user cannot destory object when its out of view
		rootSig = signature;
		//make this dynamic enough to accommodate instancing
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
		PSOdesc.sampleCount = RHI::SampleMode::x2;
		for (uint32_t depthMode = 0; depthMode < desc->numDepthStencilModes; depthMode++)
		{
			PSOdesc.depthStencilMode = desc->DepthStencilModes[depthMode];
			for (uint32_t rasterizerMode = 0; rasterizerMode < desc->numRasterizerModes; rasterizerMode++)
			{
				PSOdesc.rasterizerMode = desc->RasterizerModes[rasterizerMode];
				for (uint32_t blendMode = 0; blendMode < desc->numBlendModes; blendMode++)
				{
					PSOdesc.blendMode = desc->BlendModes[blendMode];
					PSOHash hash;
					EncodePso(hash, &PSOdesc);
#if _DEBUG
					auto h1 = PSOComparsionState(&hash);
					if (XXH64(&h1, sizeof(Pistachio::PSOHash), 10) != XXH64(&hash, sizeof(Pistachio::PSOHash), 10))
					{
						PT_CORE_WARN("{0} A disabled state still had valid configuration, this config would be zeroed during comparisons", __FUNCTION__);
					}
#endif
					RHI::PipelineStateObject* pso;
					RendererBase::Getd3dDevice()->CreatePipelineStateObject(&PSOdesc, &pso);
					PSOs[hash] = pso;
				}
			}
		}
		currentPSO = PSOs.begin()->first;
	}
	void Shader::CreateStack(ShaderCreateDesc* desc)
	{
		VS = desc->VS;
		PS = desc->PS;
		HS = desc->HS;
		GS = desc->GS;
		DS = desc->DS;
		mode = desc->shaderMode;
		CreateRootSignature();
		Initialize(desc, rootSig);
		rootSig->Release();//we can safely release this reference to the RS because CreateStackRs holds one
	}

	void Shader::CreateSetInfos(RHI::ShaderReflection* VSreflection, RHI::ShaderReflection* PSreflection)
	{
		FillSetInfo(VSreflection, m_VSinfo);
		FillSetInfo(PSreflection, m_PSinfo);
	}

	void Shader::FillSetInfo(RHI::ShaderReflection* reflection, ShaderSetInfos& info)
	{
		if (reflection)
		{
			uint32_t numSets = reflection->GetNumDescriptorSets();
			std::vector<RHI::SRDescriptorSet> sets(numSets);
			reflection->GetAllDescriptorSets(sets.data());
			info.sets.reserve(numSets);
			for (uint32_t i = 0; i < numSets; i++)
			{
				auto& set = info.sets.emplace_back();
				set.setIndex = sets[i].setIndex;
				set.set = nullptr;
				std::vector<RHI::SRDescriptorBinding> bindings(sets[i].bindingCount);
				reflection->GetDescriptorSetBindings(&sets[i], bindings.data());
				for (uint32_t j = 0; j < sets[i].bindingCount; j++)
				{
					set.count.push_back(bindings[j].count);
					set.slot.push_back(bindings[j].bindingSlot);
					set.stage.push_back(RHI::ShaderStage::Vertex);//todo
					set.type.push_back(bindings[j].resourceType);
				}
			}
		}
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
		if (VS.data)
		{
			if (mode == RHI::ShaderMode::File)
				RHI::ShaderReflection::CreateFromFile(VS.data, &VSreflection);
			else
				RHI::ShaderReflection::CreateFromMemory(VS.data, VS.size, &VSreflection);
			uint32_t numSets = VSreflection->GetNumDescriptorSets();
			std::vector<RHI::SRDescriptorSet> sets(numSets);
			VSreflection->GetAllDescriptorSets(sets.data());
			for (uint32_t i = 0; i < numSets; i++)
			{
				RHI::RootParameterDesc desc;

				desc.type = RHI::RootParameterType::DescriptorTable;
				desc.descriptorTable.setIndex = sets[i].setIndex;
				desc.descriptorTable.numDescriptorRanges = sets[i].bindingCount;
				desc.descriptorTable.ranges = (RHI::DescriptorRange*)rangeOffset;

				std::vector<RHI::SRDescriptorBinding> bindings(sets[i].bindingCount);
				VSreflection->GetDescriptorSetBindings(&sets[i], bindings.data());

				//fill out all the ranges (descriptors)
				for (uint32_t j = 0; j < sets[i].bindingCount; j++)
				{
					RHI::DescriptorRange range;
					range.BaseShaderRegister = bindings[j].bindingSlot;
					range.numDescriptors = bindings[j].count;
					range.type = bindings[j].resourceType;
					range.stage = RHI::ShaderStage::Vertex;
					ranges.push_back(range);
					rangeOffset++;
				}
				rootParams.push_back(desc);
			}
			CreateSetInfos(VSreflection, 0);
			VSreflection->Release();
		}
		if (PS.data)
		{
			if (mode == RHI::ShaderMode::File)
				RHI::ShaderReflection::CreateFromFile(PS.data, &PSreflection);
			else
				RHI::ShaderReflection::CreateFromMemory(PS.data, PS.size, &PSreflection);
			uint32_t numSets = PSreflection->GetNumDescriptorSets();
			std::vector<RHI::SRDescriptorSet> sets(numSets);
			PSreflection->GetAllDescriptorSets(sets.data());
			for (uint32_t i = 0; i < numSets; i++)
			{
				RHI::RootParameterDesc desc;
				desc.descriptorTable.setIndex = sets[i].setIndex;
				desc.type = RHI::RootParameterType::DescriptorTable;
				desc.descriptorTable.numDescriptorRanges = sets[i].bindingCount;
				desc.descriptorTable.ranges = (RHI::DescriptorRange*)rangeOffset;

				std::vector<RHI::SRDescriptorBinding> bindings(sets[i].bindingCount);
				PSreflection->GetDescriptorSetBindings(&sets[i], bindings.data());

				//fill out all the ranges (descriptors)
				for (uint32_t j = 0; j < sets[i].bindingCount; j++)
				{
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
			CreateSetInfos( 0,PSreflection);
			PSreflection->Release();
		}
		for (auto& param : rootParams)
		{
			param.descriptorTable.ranges = &ranges[(size_t)param.descriptorTable.ranges];
		}
		RSdesc.numRootParameters = (uint32_t)rootParams.size();
		RSdesc.rootParameters = rootParams.data();
		numLayouts = (uint32_t)rootParams.size();
		layouts = new RHI::DescriptorSetLayout*[numLayouts];
		RendererBase::Getd3dDevice()->CreateRootSignature(&RSdesc, &rootSig, layouts);
	}



	void SetInfo::UpdateBufferBinding(RHI::Buffer* buff, uint32_t offset, uint32_t size, RHI::DescriptorType type, uint32_t slot)
	{
		RHI::DescriptorBufferInfo info;
		info.buffer = buff;
		info.offset = offset;
		info.range = size;
		RHI::DescriptorSetUpdateDesc updateDesc;
		updateDesc.arrayIndex = 0;
		updateDesc.binding = slot;
		updateDesc.numDescriptors = 1;
		updateDesc.type = type;
		updateDesc.bufferInfos = &info;
		RendererBase::Getd3dDevice()->UpdateDescriptorSets(1, &updateDesc, set);
	}

	void SetInfo::UpdateBufferBinding(BufferBindingUpdateDesc* desc, uint32_t slot)
	{
		RHI::DescriptorBufferInfo info;
		info.buffer = desc->buffer;
		info.offset = desc->offset;
		info.range = desc->size;
		RHI::DescriptorSetUpdateDesc updateDesc;
		updateDesc.arrayIndex = 0;
		updateDesc.binding = slot;
		updateDesc.numDescriptors = 1;
		updateDesc.type = desc->type;
		updateDesc.bufferInfos = &info;
		RendererBase::Getd3dDevice()->UpdateDescriptorSets(1, &updateDesc, set);
	}
	void SetInfo::UpdateTextureBinding(RHI::TextureView* desc, uint32_t slot, RHI::DescriptorType type)
	{
		RHI::DescriptorTextureInfo info;
		info.texture = desc;
		RHI::DescriptorSetUpdateDesc updateDesc;
		updateDesc.arrayIndex = 0;
		updateDesc.binding = slot;
		updateDesc.numDescriptors = 1;
		updateDesc.type = type;
		updateDesc.textureInfos = &info;
		RendererBase::Getd3dDevice()->UpdateDescriptorSets(1, &updateDesc, set);
	}
	void SetInfo::UpdateSamplerBinding(SamplerHandle handle, uint32_t slot)
	{
		RHI::DescriptorSamplerInfo info;
		info.heapHandle = RendererBase::GetCPUHandle(handle);
		RHI::DescriptorSetUpdateDesc updateDesc;
		updateDesc.arrayIndex = 0;
		updateDesc.binding = slot;
		updateDesc.numDescriptors = 1;
		updateDesc.type = RHI::DescriptorType::Sampler;
		updateDesc.samplerInfos = &info;
		RendererBase::Getd3dDevice()->UpdateDescriptorSets(1, &updateDesc, set);
	}

	bool PSOHash::operator==(const PSOHash& hash) const
	{
		
		return std::hash<PSOHash>()(*this) == std::hash<PSOHash>()(hash);
	}


	ComputeShader* ComputeShader::Create(const RHI::ShaderCode& code, RHI::ShaderMode mode)
	{
		ComputeShader* shader = new ComputeShader;
		shader->CreateRootSignature(code, mode);
		RHI::ComputePipelineDesc desc;
		desc.CS = code;
		desc.mode = mode;
		desc.rootSig = shader->rSig;
		RendererBase::device->CreateComputePipeline(&desc, &shader->pipeline);
		return shader;
	}

	ComputeShader::~ComputeShader()
	{
		for (uint32_t i = 0; i < numLayouts; i++)
		{
			layouts[i]->Release();
		}
		rSig->Release();
		delete[] layouts;
	}

	void ComputeShader::Bind(RHI::GraphicsCommandList* list)
	{
		//set the root signature too
		list->SetComputePipeline(pipeline);
	}

	void ComputeShader::GetShaderBinding(SetInfo& info, uint32_t setIndex)
	{
		uint32_t index = UINT32_MAX;
		for (uint32_t i = 0; i < m_info.sets.size(); i++)
		{
			if (m_info.sets[i].setIndex == setIndex) index = i;
		}
		info = m_info.sets[index];
		RendererBase::device->CreateDescriptorSets(RendererBase::heap, 1, layouts[index], &info.set);
	}

	void ComputeShader::ApplyShaderBinding(RHI::GraphicsCommandList* list, const SetInfo& info)
	{
		list->BindComputeDescriptorSet(rSig, info.set, info.setIndex);
	}

	ComputeShader* ComputeShader::CreateWithRs(const RHI::ShaderCode& code, RHI::ShaderMode mode, RHI::RootSignature* rSig)
	{
		rSig->Hold();
		ComputeShader* shader = new ComputeShader;
		shader->rSig = rSig;
		RHI::ShaderReflection* CSReflection;
		if (mode == RHI::ShaderMode::File) RHI::ShaderReflection::CreateFromFile(code.data, &CSReflection);
		else RHI::ShaderReflection::CreateFromMemory(code.data, code.size, &CSReflection);
		shader->CreateSetInfos(CSReflection);
		CSReflection->Release();
		RHI::ComputePipelineDesc desc;
		desc.CS = code;
		desc.mode = mode;
		desc.rootSig = shader->rSig;
		RendererBase::device->CreateComputePipeline(&desc, &shader->pipeline);
		return shader;
	}

	void ComputeShader::CreateRootSignature(const RHI::ShaderCode& code, RHI::ShaderMode mode)
	{
		RHI::ShaderReflection* CSReflection;
		if (mode == RHI::ShaderMode::File) RHI::ShaderReflection::CreateFromFile(code.data, &CSReflection);
		else RHI::ShaderReflection::CreateFromMemory(code.data, code.size, &CSReflection);
		uint32_t numSets = CSReflection->GetNumDescriptorSets();
		std::vector<RHI::SRDescriptorSet> sets(numSets);
		CSReflection->GetAllDescriptorSets(sets.data());
		std::vector<std::vector<RHI::DescriptorRange>> ranges_vec;
		std::vector<RHI::RootParameterDesc> rootParams;
		for (uint32_t i = 0; i < numSets; i++)
		{
			RHI::RootParameterDesc desc;

			desc.type = RHI::RootParameterType::DescriptorTable;
			desc.descriptorTable.setIndex = sets[i].setIndex;
			desc.descriptorTable.numDescriptorRanges = sets[i].bindingCount;

			std::vector<RHI::SRDescriptorBinding> bindings(sets[i].bindingCount);
			CSReflection->GetDescriptorSetBindings(&sets[i], bindings.data());
			std::vector<RHI::DescriptorRange>& ranges = ranges_vec.emplace_back();
			//fill out all the ranges (descriptors)
			for (uint32_t j = 0; j < sets[i].bindingCount; j++)
			{
				RHI::DescriptorRange& range = ranges.emplace_back();
				range.BaseShaderRegister = bindings[j].bindingSlot;
				range.numDescriptors = bindings[j].count;
				range.type = bindings[j].resourceType;
				range.stage = RHI::ShaderStage::Compute;
			}
			desc.descriptorTable.ranges = ranges.data();
			rootParams.push_back(desc);
			RHI::RootSignatureDesc rsDesc;
			rsDesc.numRootParameters = (uint32_t)rootParams.size();
			rsDesc.rootParameters = rootParams.data();
			numLayouts = (uint32_t)rootParams.size();
			layouts = new RHI::DescriptorSetLayout*[numLayouts];
			RendererBase::device->CreateRootSignature(&rsDesc, &rSig, layouts);
		}
		CreateSetInfos(CSReflection);
		CSReflection->Release();

	}

	void ComputeShader::CreateSetInfos(RHI::ShaderReflection* reflection)
	{
		uint32_t numSets = reflection->GetNumDescriptorSets();
		std::vector<RHI::SRDescriptorSet> sets(numSets);
		reflection->GetAllDescriptorSets(sets.data());
		m_info.sets.reserve(numSets);
		for (uint32_t i = 0; i < numSets; i++)
		{
			auto& set = m_info.sets.emplace_back();
			set.setIndex = sets[i].setIndex;
			set.set = nullptr;
			std::vector<RHI::SRDescriptorBinding> bindings(sets[i].bindingCount);
			reflection->GetDescriptorSetBindings(&sets[i], bindings.data());
			for (uint32_t j = 0; j < sets[i].bindingCount; j++)
			{
				set.count.push_back(bindings[j].count);
				set.slot.push_back(bindings[j].bindingSlot);
				set.stage.push_back(RHI::ShaderStage::Compute);
				set.type.push_back(bindings[j].resourceType);
			}
		}
	}

}

void Pistachio::Helpers::ZeroAndFillShaderDesc(ShaderCreateDesc* desc, const char* VS, const char* PS, uint32_t numRenderTargets,  uint32_t numDSModes, RHI::DepthStencilMode* dsMode, uint32_t numBlendModes, RHI::BlendMode* blendModes, uint32_t numRasterizerModes, RHI::RasterizerMode* rsModes, const char* GS,const char* HS,   const char* DS)
{
	memset(desc, 0, sizeof(ShaderCreateDesc));
	desc->VS = RHI::ShaderCode{ (char*)VS, 0 };
	desc->PS = RHI::ShaderCode{ (char*)PS, 0 };
	desc->GS = RHI::ShaderCode{ (char*)GS, 0 };
	desc->HS = RHI::ShaderCode{ (char*)HS, 0 };
	desc->DS = RHI::ShaderCode{ (char*)DS, 0 };
	desc->NumRenderTargets = numRenderTargets;
	desc->numDepthStencilModes = numDSModes;
	desc->numBlendModes = numBlendModes;
	desc->DepthStencilModes = dsMode;
	desc->BlendModes = blendModes;
	desc->RasterizerModes = rsModes;
	desc->numRasterizerModes = numRasterizerModes;
	desc->shaderMode = RHI::ShaderMode::File;
}

void Pistachio::Helpers::FillDepthStencilMode(RHI::DepthStencilMode* mode, bool depthEnabled, RHI::DepthWriteMask mask, RHI::ComparisonFunc depthFunc, bool stencilEnable, uint8_t stencilReadMask, uint8_t stencilWriteMask, RHI::DepthStencilOp* front, RHI::DepthStencilOp* back)
{
	mode->DepthEnable = depthEnabled;
	mode->DepthWriteMask = mask;
	mode->DepthFunc = depthFunc;
	if(front) mode->FrontFace = *front;
	if(back) mode->BackFace = *back;
	mode->StencilEnable = stencilEnable;
	mode->StencilReadMask = stencilReadMask;
	mode->StencilWriteMask = stencilWriteMask;
}

void Pistachio::Helpers::BlendModeDisabledBlend(RHI::BlendMode* mode)
{
	mode->BlendAlphaToCoverage = false;
	mode->IndependentBlend = true;
	mode->blendDescs[0].blendEnable = false;
}

void Pistachio::Helpers::FillRaseterizerMode(RHI::RasterizerMode* mode, RHI::FillMode fillMode, RHI::CullMode cullMode, RHI::PrimitiveTopology topology, bool multiSample, bool antiAliasedLine, bool conservativeRaster, int depthBias, float depthBiasClamp, float ssDepthBias, bool depthClip)
{
	mode->AntialiasedLineEnable = antiAliasedLine;
	mode->conservativeRaster = conservativeRaster;
	mode->cullMode = cullMode;
	mode->depthBias = depthBias;
	mode->depthBiasClamp = depthBiasClamp;
	mode->depthClipEnable = depthClip;
	mode->fillMode = fillMode;
	mode->multisampleEnable = multiSample;
	mode->slopeScaledDepthBias = ssDepthBias;
	mode->topology = topology;
}

void Pistachio::Helpers::FillDescriptorSetRootParam(RHI::RootParameterDesc* rpDesc, uint32_t numRanges, uint32_t setIndex, RHI::DescriptorRange* ranges)
{
	rpDesc->type = RHI::RootParameterType::DescriptorTable;
	rpDesc->descriptorTable.numDescriptorRanges = numRanges;
	rpDesc->descriptorTable.ranges = ranges;
	rpDesc->descriptorTable.setIndex = setIndex;
}

void Pistachio::Helpers::FillDescriptorRange(RHI::DescriptorRange* range, uint32_t numDescriptors, uint32_t shaderRegister, RHI::ShaderStage stage, RHI::DescriptorType type)
{
	range->BaseShaderRegister = shaderRegister;
	range->numDescriptors = numDescriptors;
	range->stage = stage;
	range->type = type;
}

void Pistachio::Helpers::FillDynamicDescriptorRootParam(RHI::RootParameterDesc* rpDesc, uint32_t setIndex, RHI::DescriptorType type, RHI::ShaderStage stage)
{
	rpDesc->type = RHI::RootParameterType::DynamicDescriptor;
	rpDesc->dynamicDescriptor.setIndex = setIndex;
	rpDesc->dynamicDescriptor.stage = stage;
	rpDesc->dynamicDescriptor.type = type;
}
