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
		for (int i = 0; i < desc->numRenderTargets; i++)
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
				resultBuffer.rtblends[i].srcColorBlendFac_logicOp = (unsigned int)desc->blendMode.blendDescs[i].LogicOp;
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
			outbuf->blendMode.blendDescs[i].LogicOp       = (RHI::LogicOp)hash->rtblends[i].srcColorBlendFac_logicOp;
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
				if(!VS.empty()) desc.VS = VS.c_str();
				if(!PS.empty()) desc.PS = PS.c_str();
				if(!GS.empty()) desc.GS = GS.c_str();
				if(!HS.empty()) desc.HS = HS.c_str();
				if(!DS.empty()) desc.DS = DS.c_str();
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

	bool PSOHash::operator==(const PSOHash& hash) const
	{
		
		return std::hash<PSOHash>()(*this) == std::hash<PSOHash>()(hash);
	}

}

