#include "FormatsAndTypes.h"
#include "PipelineStateObject.h"
#include "Pistachio/Renderer/Renderer.h"
#include "Ptr.h"
#include "RootSignature.h"
#include "ShaderReflect.h"
#include "Util/FormatUtils.h"
#include "ptpch.h"
#include "Shader.h"
#include "RendererBase.h"
#include <optional>
#include <ranges>
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

namespace Pistachio {

	template<> RHI::DepthStencilMode PSOHash::Into<RHI::DepthStencilMode>()
	{
		RHI::DepthStencilMode mode;
		mode.BackFace.DepthfailOp = (RHI::StencilOp)back_depthFail;
		mode.BackFace.failOp	   = (RHI::StencilOp)back_fail;
		mode.BackFace.passOp	   = (RHI::StencilOp)back_pass;
		mode.BackFace.Stencilfunc = (RHI::ComparisonFunc)back_func;
		mode.FrontFace.DepthfailOp = (RHI::StencilOp)front_depthFail;
		mode.FrontFace.failOp	    = (RHI::StencilOp)front_fail;
		mode.FrontFace.passOp	    = (RHI::StencilOp)front_pass;
		mode.FrontFace.Stencilfunc = (RHI::ComparisonFunc)front_func;
		mode.DepthEnable = depthEnable;
		mode.DepthFunc = (RHI::ComparisonFunc)depthFunc;
		mode.DepthWriteMask = (RHI::DepthWriteMask)depthWriteMask;
		mode.StencilEnable = stencilEnable;
		mode.StencilReadMask = stencilReadMask;
		mode.StencilWriteMask = stencilWriteMask;
		return mode;
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
		if (VS.data) free(VS.mut_data);
		if (PS.data) free(PS.mut_data);
		if (HS.data) free(HS.mut_data);
		if (GS.data) free(GS.mut_data);
		if (DS.data) free(DS.mut_data);
	}

	Shader* Shader::Create(const ShaderCreateDesc& desc, std::span<const uint32_t> dynamic_sets, std::optional<uint32_t> push_block)
	{
		Shader* shader = new Shader;
		shader->CreateStack(desc, dynamic_sets, push_block);
		return shader;
	}


	void Shader::Bind(RHI::Weak<RHI::GraphicsCommandList> list)
	{
		list->SetRootSignature(rootSig);
		list->SetPipelineState(PSOs[currentPSO]);
	}

	void Shader::GetDepthStencilMode(RHI::DepthStencilMode& mode)
	{
		mode = currentPSO.Into<RHI::DepthStencilMode>();	
	}

	uint32_t Shader::SetDepthStencilMode(const RHI::DepthStencilMode& mode, ShaderModeSetFlags flags)
	{
		PSOHash newHash = currentPSO;
		newHash.back_depthFail      = (std::uint32_t)mode.BackFace.DepthfailOp  ;
		newHash.back_fail           = (std::uint32_t)mode.BackFace.failOp       ;
		newHash.back_pass           = (std::uint32_t)mode.BackFace.passOp       ;
		newHash.back_func           = (std::uint32_t)mode.BackFace.Stencilfunc  ;
		newHash.front_depthFail     = (std::uint32_t)mode.FrontFace.DepthfailOp ;
		newHash.front_fail          = (std::uint32_t)mode.FrontFace.failOp      ;
		newHash.front_pass          = (std::uint32_t)mode.FrontFace.passOp      ;
		newHash.front_func          = (std::uint32_t)mode.FrontFace.Stencilfunc ;
		newHash.depthEnable         =                mode.DepthEnable           ;
		newHash.depthFunc           = (std::uint32_t)mode.DepthFunc             ;
		newHash.depthWriteMask      = (std::uint32_t)mode.DepthWriteMask        ;
		newHash.stencilEnable       =                mode.StencilEnable         ;
		newHash.stencilReadMask     =                mode.StencilReadMask       ;
		newHash.stencilWriteMask    =                mode.StencilWriteMask      ;
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
				currentPSO = newHash;
				PSOs[newHash] = RendererBase::Get3dDevice()->CreatePipelineStateObject(desc).value();
				return 0;
			}
			return  1;
		}
	}



	void Shader::GetShaderBinding(SetInfo& info, uint32_t setIndex)
	{
		uint32_t index = UINT32_MAX;
		for (uint32_t i = 0; i < m_infos.sets.size(); i++)
		{
			if (m_infos.sets[i].setIndex == setIndex) index = i;
		}
		info = m_infos.sets[index];
		info.set = RendererBase::CreateDescriptorSet(layouts[index]);
	}

	void Shader::ApplyBinding(RHI::Weak<RHI::GraphicsCommandList> list,const SetInfo& info)
	{
		list->BindDescriptorSet(info.set, info.setIndex);
	}

	
	void Shader::Initialize(const ShaderCreateDesc& desc, RHI::Ptr<RHI::RootSignature> signature)
	{
		RHI::PipelineStateObjectDesc PSOdesc;
		//intialize state that doesn't change
		PSOdesc.VS = desc.VS;
		PSOdesc.PS = desc.PS;
		PSOdesc.HS = desc.HS;
		PSOdesc.GS = desc.GS;
		PSOdesc.DS = desc.DS;
		PSOdesc.shaderMode = desc.shaderMode;
		mode = desc.shaderMode;
		//only suppport 6 rtvs
		PSOdesc.RTVFormats[0] = desc.RTVFormats[0];
		PSOdesc.RTVFormats[1] = desc.RTVFormats[1];
		PSOdesc.RTVFormats[2] = desc.RTVFormats[2];
		PSOdesc.RTVFormats[3] = desc.RTVFormats[3];
		PSOdesc.RTVFormats[4] = desc.RTVFormats[4];
		PSOdesc.RTVFormats[5] = desc.RTVFormats[5];
		rtvFormats[0] = desc.RTVFormats[0];
		rtvFormats[1] = desc.RTVFormats[1];
		rtvFormats[2] = desc.RTVFormats[2];
		rtvFormats[3] = desc.RTVFormats[3];
		rtvFormats[4] = desc.RTVFormats[4];
		rtvFormats[5] = desc.RTVFormats[5];
		PSOdesc.numRenderTargets = desc.NumRenderTargets;
		numRenderTargets = desc.NumRenderTargets;
		PSOdesc.DSVFormat = desc.DSVFormat;
		dsvFormat = desc.DSVFormat;
		PSOdesc.rootSig = signature;
		rootSig = signature;
		//make this dynamic enough to accommodate instancing
		InputBindingDescription = new RHI::InputBindingDesc;
		InputBindingDescription->inputRate = RHI::InputRate::Vertex;
		InputBindingDescription->stride = desc.InputDescription[desc.numInputs - 1].Offset + RHI::Util::GetFormatBPP(desc.InputDescription[desc.numInputs - 1].Format);
		InputElementDescription = new RHI::InputElementDesc[desc.numInputs];
		numInputBindings = 1;
		numInputElements = desc.numInputs;
		for (uint32_t i = 0; i < desc.numInputs; i++)
		{
			InputElementDescription[i].alignedByteOffset = desc.InputDescription[i].Offset;
			InputElementDescription[i].format = desc.InputDescription[i].Format;
			InputElementDescription[i].inputSlot = 0;
			InputElementDescription[i].location = i;
		}
		PSOdesc.inputBindings = InputBindingDescription;
		PSOdesc.numInputBindings = 1;
		PSOdesc.inputElements = InputElementDescription;
		PSOdesc.numInputElements = desc.numInputs;
		PSOdesc.sampleCount = RHI::SampleMode::x2;
		for (uint32_t depthMode = 0; depthMode < desc.numDepthStencilModes; depthMode++)
		{
			PSOdesc.depthStencilMode = desc.DepthStencilModes[depthMode];
			for (uint32_t rasterizerMode = 0; rasterizerMode < desc.numRasterizerModes; rasterizerMode++)
			{
				PSOdesc.rasterizerMode = desc.RasterizerModes[rasterizerMode];
				for (uint32_t blendMode = 0; blendMode < desc.numBlendModes; blendMode++)
				{
					PSOdesc.blendMode = desc.BlendModes[blendMode];
					PSOHash hash;
					EncodePso(hash, &PSOdesc);
#if _DEBUG
					auto h1 = PSOComparsionState(&hash);
					if (XXH64(&h1, sizeof(Pistachio::PSOHash), 10) != XXH64(&hash, sizeof(Pistachio::PSOHash), 10))
					{
						PT_CORE_WARN("{0} A disabled state still had valid configuration, this config would be zeroed during comparisons", __FUNCTION__);
					}
#endif
					PSOs[hash] = RendererBase::Get3dDevice()->CreatePipelineStateObject(PSOdesc).value();
				}
			}
		}
		currentPSO = PSOs.begin()->first;
	}
	void Shader::CreateStack(const ShaderCreateDesc& desc, std::span<const uint32_t> dynamic_sets, std::optional<uint32_t> push_block)
	{
		VS = desc.VS;
		PS = desc.PS;
		HS = desc.HS;
		GS = desc.GS;
		DS = desc.DS;
		mode = desc.shaderMode;
		CreateRootSignature(dynamic_sets, push_block);
		Initialize(desc, rootSig);
	}

	void Shader::FillSetInfo(RHI::RootSignatureDesc& dsc)
	{
		for(auto i : std::views::iota(dsc.rootParameters, dsc.rootParameters + dsc.numRootParameters))
		{
			if(i->type == RHI::RootParameterType::DescriptorTable)
			{
				auto& s = m_infos.sets.emplace_back();
				s.setIndex = i->descriptorTable.setIndex;
				s.set = nullptr;
				s.count.reserve(i->descriptorTable.numDescriptorRanges);
				s.slot.reserve(i->descriptorTable.numDescriptorRanges);
				s.stage.reserve(i->descriptorTable.numDescriptorRanges);
				s.type.reserve(i->descriptorTable.numDescriptorRanges);
				for(auto j : std::views::iota(i->descriptorTable.ranges, i->descriptorTable.ranges + i->descriptorTable.numDescriptorRanges))
				{
					s.count.push_back(j->numDescriptors);
					s.slot.push_back(j->BaseShaderRegister);
					s.stage.push_back(j->stage);
					s.type.push_back(j->type);
				}
			}
		}
	}

	void Shader::CreateRootSignature(std::span<const uint32_t> dynamic_sets, std::optional<uint32_t> push_block)
	{
		//RHI's reflection API will take of this for us
		std::vector<RHI::Ptr<RHI::ShaderReflection>> reflections;
		if(mode == RHI::File)
		{
			if(VS.data) reflections.push_back(RHI::ShaderReflection::CreateFromFile(VS.data).value());
			if(PS.data) reflections.push_back(RHI::ShaderReflection::CreateFromFile(PS.data).value());
			if(GS.data) reflections.push_back(RHI::ShaderReflection::CreateFromFile(GS.data).value());
			if(HS.data) reflections.push_back(RHI::ShaderReflection::CreateFromFile(HS.data).value());
			if(DS.data) reflections.push_back(RHI::ShaderReflection::CreateFromFile(DS.data).value());
		}
		else if (mode == RHI::Memory)
		{
			if(VS.data) reflections.push_back(RHI::ShaderReflection::CreateFromMemory(VS.data, VS.size).value());
			if(PS.data) reflections.push_back(RHI::ShaderReflection::CreateFromMemory(PS.data, PS.size).value());
			if(GS.data) reflections.push_back(RHI::ShaderReflection::CreateFromMemory(GS.data, GS.size).value());
			if(HS.data) reflections.push_back(RHI::ShaderReflection::CreateFromMemory(HS.data, HS.size).value());
			if(DS.data) reflections.push_back(RHI::ShaderReflection::CreateFromMemory(DS.data, DS.size).value());
		}
		auto[rsd,_1,_2] = RHI::ShaderReflection::FillRootSignatureDesc(reflections, dynamic_sets, push_block);
		layouts.resize(rsd.numRootParameters);
		FillSetInfo(rsd);
		rootSig = RendererBase::Get3dDevice()->CreateRootSignature(&rsd, layouts.data()).value();
	}



	void SetInfo::UpdateBufferBinding(RHI::Weak<RHI::Buffer> buff, uint32_t offset, uint32_t size, RHI::DescriptorType type, uint32_t slot)
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
		RendererBase::Get3dDevice()->UpdateDescriptorSet(1, &updateDesc, set);
	}

	void SetInfo::UpdateBufferBinding(const BufferBindingUpdateDesc& desc, uint32_t slot)
	{
		RHI::DescriptorBufferInfo info;
		info.buffer = desc.buffer;
		info.offset = desc.offset;
		info.range = desc.size;
		RHI::DescriptorSetUpdateDesc updateDesc;
		updateDesc.arrayIndex = 0;
		updateDesc.binding = slot;
		updateDesc.numDescriptors = 1;
		updateDesc.type = desc.type;
		updateDesc.bufferInfos = &info;
		RendererBase::Get3dDevice()->UpdateDescriptorSet(1, &updateDesc, set);
	}
	void SetInfo::UpdateTextureBinding(RHI::Weak<RHI::TextureView> desc, uint32_t slot, RHI::DescriptorType type)
	{
		RHI::DescriptorTextureInfo info;
		info.texture = desc;
		RHI::DescriptorSetUpdateDesc updateDesc;
		updateDesc.arrayIndex = 0;
		updateDesc.binding = slot;
		updateDesc.numDescriptors = 1;
		updateDesc.type = type;
		updateDesc.textureInfos = &info;
		RendererBase::Get3dDevice()->UpdateDescriptorSet(1, &updateDesc, set);
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
		RendererBase::Get3dDevice()->UpdateDescriptorSet(1, &updateDesc, set);
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
		shader->pipeline = RendererBase::device->CreateComputePipeline(desc).value();
		return shader;
	}


	void ComputeShader::Bind(RHI::Weak<RHI::GraphicsCommandList> list)
	{
		//set the root signature too
		list->SetRootSignature(rSig);
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
		info.set = RendererBase::device->CreateDescriptorSets(RendererBase::heap, 1, &layouts[index]).value()[0];
	}

	void ComputeShader::ApplyShaderBinding(RHI::Weak<RHI::GraphicsCommandList> list, const SetInfo& info)
	{
		list->BindComputeDescriptorSet(info.set, info.setIndex);
	}

	ComputeShader* ComputeShader::CreateWithRs(const RHI::ShaderCode& code, RHI::ShaderMode mode, RHI::Ptr<RHI::RootSignature> rSig)
	{
		ComputeShader* shader = new ComputeShader;
		shader->rSig = rSig;
		RHI::Ptr<RHI::ShaderReflection> CSReflection;
		if (mode == RHI::ShaderMode::File) CSReflection = RHI::ShaderReflection::CreateFromFile(code.data).value();
		else CSReflection = RHI::ShaderReflection::CreateFromMemory(code.data, code.size).value();
		shader->CreateSetInfos(CSReflection);
		RHI::ComputePipelineDesc desc;
		desc.CS = code;
		desc.mode = mode;
		desc.rootSig = shader->rSig;
		shader->pipeline = RendererBase::device->CreateComputePipeline(desc).value();
		return shader;
	}

	void ComputeShader::CreateRootSignature(const RHI::ShaderCode& code, RHI::ShaderMode mode)
	{
		std::array<RHI::Ptr<RHI::ShaderReflection>,1> CSReflection;
		if (mode == RHI::ShaderMode::File) CSReflection[0] = RHI::ShaderReflection::CreateFromFile(code.data).value();
		else CSReflection[0] = RHI::ShaderReflection::CreateFromMemory(code.data, code.size).value();
		auto[rsd, _1, _2] = RHI::ShaderReflection::FillRootSignatureDesc(CSReflection, {}, std::nullopt);
		layouts.resize(rsd.numRootParameters);
		rSig = RendererBase::device->CreateRootSignature(&rsd, layouts.data()).value();
		CreateSetInfos(CSReflection[0]);

	}

	void ComputeShader::CreateSetInfos(RHI::Weak<RHI::ShaderReflection> reflection)
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

void Pistachio::Helpers::ZeroAndFillShaderDesc(ShaderCreateDesc& desc, const char* VS, const char* PS, uint32_t numRenderTargets,  uint32_t numDSModes, RHI::DepthStencilMode* dsMode, uint32_t numBlendModes, RHI::BlendMode* blendModes, uint32_t numRasterizerModes, RHI::RasterizerMode* rsModes, const char* GS,const char* HS,   const char* DS)
{
	memset(&desc, 0, sizeof(ShaderCreateDesc));
	desc.VS = RHI::ShaderCode{ {(char*)VS}, 0 };
	desc.PS = RHI::ShaderCode{ {(char*)PS}, 0 };
	desc.GS = RHI::ShaderCode{ {(char*)GS}, 0 };
	desc.HS = RHI::ShaderCode{ {(char*)HS}, 0 };
	desc.DS = RHI::ShaderCode{ {(char*)DS}, 0 };
	desc.NumRenderTargets = numRenderTargets;
	desc.numDepthStencilModes = numDSModes;
	desc.numBlendModes = numBlendModes;
	desc.DepthStencilModes = dsMode;
	desc.BlendModes = blendModes;
	desc.RasterizerModes = rsModes;
	desc.numRasterizerModes = numRasterizerModes;
	desc.shaderMode = RHI::ShaderMode::File;
}

void Pistachio::Helpers::FillDepthStencilMode(RHI::DepthStencilMode& mode, bool depthEnabled, RHI::DepthWriteMask mask, RHI::ComparisonFunc depthFunc, bool stencilEnable, uint8_t stencilReadMask, uint8_t stencilWriteMask, RHI::DepthStencilOp* front, RHI::DepthStencilOp* back)
{
	mode.DepthEnable = depthEnabled;
	mode.DepthWriteMask = mask;
	mode.DepthFunc = depthFunc;
	if(front) mode.FrontFace = *front;
	if(back) mode.BackFace = *back;
	mode.StencilEnable = stencilEnable;
	mode.StencilReadMask = stencilReadMask;
	mode.StencilWriteMask = stencilWriteMask;
}

void Pistachio::Helpers::BlendModeDisabledBlend(RHI::BlendMode& mode)
{
	mode.BlendAlphaToCoverage = false;
	mode.IndependentBlend = true;
	mode.blendDescs[0].blendEnable = false;
}

void Pistachio::Helpers::FillRaseterizerMode(RHI::RasterizerMode& mode, RHI::FillMode fillMode, RHI::CullMode cullMode, RHI::PrimitiveTopology topology, bool multiSample, bool antiAliasedLine, bool conservativeRaster, int depthBias, float depthBiasClamp, float ssDepthBias, bool depthClip)
{
	mode.AntialiasedLineEnable = antiAliasedLine;
	mode.conservativeRaster = conservativeRaster;
	mode.cullMode = cullMode;
	mode.depthBias = depthBias;
	mode.depthBiasClamp = depthBiasClamp;
	mode.depthClipEnable = depthClip;
	mode.fillMode = fillMode;
	mode.multisampleEnable = multiSample;
	mode.slopeScaledDepthBias = ssDepthBias;
	mode.topology = topology;
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
