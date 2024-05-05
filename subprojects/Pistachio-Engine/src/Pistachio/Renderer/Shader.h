#pragma once
#include "../Core.h"
#include "Core/Device.h"
#include "RendererID_t.h"
#include "RendererBase.h"
#include "Core/ShaderReflect.h"
#include "Core/DescriptorHeap.h"
#include "Core/Device.h"
#include "Pistachio/Core/Math.h"
#include "xxhash.h"
namespace Pistachio
{
	struct PSOHash;
	extern PSOHash PSOComparsionState(const PSOHash* pso);
	
	struct PSOHash
	{
		float depthBiasClamp;
		float slopeScaledDepthBias;
		int depthBias;
		std::uint8_t stencilReadMask : 8;

		std::uint8_t stencilWriteMask : 8;

		std::uint8_t depthFunc : 3;
		std::uint8_t front_fail : 3;
		std::uint8_t depthEnable : 1;
		std::uint8_t depthWriteMask : 1;

		std::uint8_t front_pass : 3;
		std::uint8_t front_depthFail : 3;
		std::uint8_t stencilEnable : 1;
		std::uint8_t fillMode : 1;

		std::uint8_t front_func : 3;
		std::uint8_t back_fail : 3;
		std::uint8_t multi_sample_count : 2;

		std::uint8_t back_depthFail : 3;
		std::uint8_t back_pass : 3;
		std::uint8_t primitiveTopologyType : 2;


		std::uint8_t back_func : 3;
		std::uint8_t cullMode : 2;
		std::uint8_t conservative_raster : 1;
		std::uint8_t blend_alpha_to_coverage : 1;
		std::uint8_t independent_blend : 1;

		std::uint8_t depthClipEnable : 1;
		std::uint8_t AntialiasedLineEnable : 1;
		//we have 3 bytes per RTBlend, which is 24 bits
		//color blends can hold more items, so they get 4 bits
		// could have just unpacked to dave space, but that would diminish flexibility way too much
		struct RTBlend {
			std::uint8_t colorBlendOp : 3;
			std::uint8_t srcColorBlendFac_logicOp : 4;
			std::uint8_t blend_or_logic : 1;
			std::uint8_t enabled : 1;
			std::uint8_t dstColorBlendFac : 4;
			std::uint8_t srcAlphaBlendFac : 3;
			std::uint8_t dstAlphaBlendFac : 3;
			std::uint8_t alphaOp : 3;
		};
		RTBlend rtblends[6];
		bool operator==(const PSOHash& hash) const;
	};
}
namespace std {
	template<>
	struct hash<Pistachio::PSOHash>
	{
		std::size_t operator()(const Pistachio::PSOHash& pso) const
		{
			Pistachio::PSOHash h1 = Pistachio::PSOComparsionState(&pso);
			if constexpr (sizeof(size_t) == 8)
			{

				return XXH64(&h1, sizeof(Pistachio::PSOHash), 10);
			}
			else
			{
				return XXH32(&h1, sizeof(Pistachio::PSOHash), 10);
			}
		}
	};
}
namespace Pistachio {

	
	enum class PISTACHIO_API LightType
	{
		Directional, Point, Spot
	};
	
	struct PISTACHIO_API MaterialStruct
	{
		Vector4 albedo = {0,0,0,0};
		float metallic= 0;
		float roughness=0;
		int ID = 0;
		int _pad = 0;
	};
	enum class PISTACHIO_API BufferLayoutFormat
	{
		FLOAT4 = 0,
		UINT4,
		INT4,
		FLOAT3,
		UINT3,
		INT3,
		FLOAT2,
		UINT2,
		INT2,
		FLOAT,
		UINT,
		INT,
	};
	struct PISTACHIO_API BufferLayout
	{
		const char* Name;
		BufferLayoutFormat Format;
		unsigned int Offset;

		BufferLayout(const char* name, BufferLayoutFormat format, unsigned int offset) : Name(name), Format(format), Offset(offset)
		{}
		BufferLayout(){}
	};
	static unsigned int BufferLayoutFormatSize(BufferLayoutFormat format)
	{
		switch (format)
		{
		case BufferLayoutFormat::FLOAT4: return 4 * 4;
		case BufferLayoutFormat::UINT4:  return 4 * 4;
		case BufferLayoutFormat::INT4:   return 4 * 4;
		case BufferLayoutFormat::FLOAT3: return 4 * 3;
		case BufferLayoutFormat::UINT3:  return 4 * 3;
		case BufferLayoutFormat::INT3:   return 4 * 3;
		case BufferLayoutFormat::FLOAT2: return 4 * 2;
		case BufferLayoutFormat::UINT2:  return 4 * 2;
		case BufferLayoutFormat::INT2:   return 4 * 2;
		case BufferLayoutFormat::FLOAT:  return 4;
		case BufferLayoutFormat::UINT:   return 4;
		case BufferLayoutFormat::INT:    return 4;
		default: return 0;
			break;
		}
	}
	
	
	struct ShaderCreateDesc
	{
		//Constant
		RHI::ShaderCode VS = {nullptr, 0};
		RHI::ShaderCode PS = {nullptr, 0};
		RHI::ShaderCode GS = {nullptr, 0};
		RHI::ShaderCode HS = {nullptr, 0};
		RHI::ShaderCode DS = {nullptr, 0};
		uint32_t NumRenderTargets = 1;	//can't be changed because this is with the pixel shader
		RHI::Format RTVFormats[8]; // make this constant too??, or make it such that a shader can be used with multipe rtv formats
		RHI::Format DSVFormat;
		BufferLayout* InputDescription = nullptr;
		uint32_t numInputs = 0;

		//Variable
		RHI::DepthStencilMode* DepthStencilModes = nullptr;
		uint32_t numDepthStencilModes = 0;
		RHI::BlendMode* BlendModes = nullptr;
		uint32_t numBlendModes = 0;
		RHI::RasterizerMode* RasterizerModes = nullptr;
		uint32_t numRasterizerModes = 0;
		RHI::ShaderMode shaderMode = RHI::ShaderMode::File;
	};
	enum class ShaderModeSetFlags
	{
		None = 0,
		AutomaticallyCreate = 1
	};
	struct BufferBindingUpdateDesc
	{
		RHI::DescriptorType type;
		RHI::Buffer* buffer;
		uint32_t offset;
		uint32_t size;
	};
	struct PISTACHIO_API SetInfo
	{
		// provide binding slot, count and type of each Binding
		std::vector<uint32_t> slot;
		std::vector<RHI::DescriptorType> type;
		std::vector<uint32_t> count;
		std::vector<RHI::ShaderStage> stage;

		std::uint32_t setIndex;
		RHI::DescriptorSet* set;
		void UpdateBufferBinding(RHI::Buffer* buff, uint32_t offset, uint32_t size, RHI::DescriptorType type, uint32_t slot);
		void UpdateBufferBinding(BufferBindingUpdateDesc* desc, uint32_t slot);
		void UpdateTextureBinding(RHI::TextureView* view, uint32_t slot, RHI::DescriptorType type = RHI::DescriptorType::SampledTexture);
		void UpdateSamplerBinding(SamplerHandle handle, uint32_t slot);
	};
	//Every Shader has one of these
	struct  ShaderSetInfos
	{
		// we have a bunch of dsets from the shader, and info on how to populate them??
		std::vector<SetInfo> sets;
		
	};
	
	class PISTACHIO_API ComputeShader
	{
	public:
		ComputeShader() {};
		~ComputeShader();
		void Bind(RHI::GraphicsCommandList* list);
		static ComputeShader* Create(const RHI::ShaderCode& code, RHI::ShaderMode mode);
		void GetShaderBinding(SetInfo& info, uint32_t setIndex);
		void ApplyShaderBinding(RHI::GraphicsCommandList* list, const SetInfo& info);
		static ComputeShader* CreateWithRs(const RHI::ShaderCode& code, RHI::ShaderMode mode, RHI::RootSignature* rSig);
	private:
		friend class ComputePass;
		void CreateRootSignature(const RHI::ShaderCode& code, RHI::ShaderMode mode);
		void CreateSetInfos(RHI::ShaderReflection* reflection);
		RHI::RootSignature* rSig;
		RHI::DescriptorSetLayout** layouts;
		RHI::ComputePipeline* pipeline;
		uint32_t numLayouts;
		ShaderSetInfos m_info;
	};
	class PISTACHIO_API Shader
	{
	public:
		Shader();
		~Shader();
		static Shader* Create(ShaderCreateDesc* desc);
		static Shader* CreateWithRs(ShaderCreateDesc* desc, RHI::RootSignature* rs,RHI::DescriptorSetLayout** layouts,uint32_t numLayouts);
		void Bind(RHI::GraphicsCommandList* list);
		void GetDepthStencilMode(RHI::DepthStencilMode* mode);
		uint32_t SetDepthStencilMode(RHI::DepthStencilMode* mode, ShaderModeSetFlags flags);
		void GetRasterizerMode(RHI::RasterizerMode* mode);
		uint32_t SetRasterizerMode(RHI::RasterizerMode* mode, ShaderModeSetFlags flags);
		void GetBlendMode(RHI::BlendMode* mode);
		void GetVSShaderBinding(SetInfo& info, uint32_t setIndex);
		void GetPSShaderBinding(SetInfo& info, uint32_t setIndex);
		void ApplyBinding(RHI::GraphicsCommandList* list, const SetInfo& info);
		RHI::RootSignature* GetRootSignature() { return rootSig; };
		uint32_t SetBlendMode(RHI::BlendMode* mode, ShaderModeSetFlags flags);
		RHI::PipelineStateObject* GetCurrentPipeline() { return PSOs[currentPSO]; }
	private:
		void Initialize(ShaderCreateDesc* desc,RHI::RootSignature* rs);
		void CreateStack(ShaderCreateDesc* desc);
		void CreateStackRs(ShaderCreateDesc* desc, RHI::RootSignature* rs, RHI::DescriptorSetLayout** layouts, uint32_t numLayouts);
		void CreateSetInfos(RHI::ShaderReflection* VSreflection, RHI::ShaderReflection* PSreflection);
		void FillSetInfo(RHI::ShaderReflection* reflection,ShaderSetInfos& info);
		void CreateRootSignature();
	private:
		friend class ShaderAsset;
		friend class Renderer;
		std::unordered_map<PSOHash, RHI::PipelineStateObject*> PSOs;
		ShaderSetInfos m_VSinfo;
		ShaderSetInfos m_PSinfo;
		RHI::RootSignature* rootSig;
		RHI::DescriptorSetLayout** layouts;
		uint32_t numLayouts;
		RHI::Format rtvFormats[6];
		RHI::Format dsvFormat;
		uint32_t numRenderTargets;
		RHI::ShaderCode VS = {};
		RHI::ShaderCode PS = {};
		RHI::ShaderCode GS = {};
		RHI::ShaderCode HS = {};
		RHI::ShaderCode DS = {};
		RHI::InputBindingDesc* InputBindingDescription;
		RHI::InputElementDesc* InputElementDescription;
		std::uint32_t numInputElements;
		std::uint32_t numInputBindings;
		PSOHash currentPSO;
		RHI::ShaderMode mode;
	};
	namespace Helpers
	{
		/*
		* Zeroes out and Fills a Shader Create Desc with a bunch of valid parameters
		* Things to note:
		*	Shader Mode is File
		*	Depth Stencil, Blend and Render Target modes are not filled
		*	Texture formats are not filled
		*	Input Description is Filled
		*/
		void ZeroAndFillShaderDesc(ShaderCreateDesc* desc,
			const char* VS = nullptr,
			const char* PS = nullptr,
			uint32_t numRenderTargets = 1,
			uint32_t numDSModes = 0,
			RHI::DepthStencilMode* dsModes = nullptr,
			uint32_t numBlendModes = 0,
			RHI::BlendMode* blendModes = nullptr,
			uint32_t numRasterizerModes = 0,
			RHI::RasterizerMode* rsModes = nullptr,
			const char* GS = nullptr,
			const char* HS = nullptr,
			const char* DS = nullptr);
		void FillDepthStencilMode(RHI::DepthStencilMode* mode,
			bool depthEnabled = true,
			RHI::DepthWriteMask mask = RHI::DepthWriteMask::All,
			RHI::ComparisonFunc depthFunc = RHI::ComparisonFunc::LessEqual,
			bool stencilEnable = false,
			uint8_t stencilReadMask = 0,
			uint8_t stencilWriteMask = 0,
			RHI::DepthStencilOp* front = nullptr,
			RHI::DepthStencilOp* back = nullptr);
		void BlendModeDisabledBlend(RHI::BlendMode* mode);
		void FillRaseterizerMode(RHI::RasterizerMode* mode,
			RHI::FillMode fillMode = RHI::FillMode::Solid,
			RHI::CullMode cullMode = RHI::CullMode::Back,
			RHI::PrimitiveTopology topology = RHI::PrimitiveTopology::TriangleList,
			bool multiSample = false,
			bool antiAliasedLine = false,
			bool conservativeRaster = false,
			int depthBias = 0,
			float depthBiasClamp = 0,
			float ssDepthBias = 0,
			bool depthClip = false);
		void FillDescriptorSetRootParam(RHI::RootParameterDesc* rpDesc, uint32_t numRanges, uint32_t setIndex, RHI::DescriptorRange* ranges);
		void FillDynamicDescriptorRootParam(RHI::RootParameterDesc* rpDesc, uint32_t setIndex, RHI::DescriptorType type, RHI::ShaderStage stage);
		void FillDescriptorRange(RHI::DescriptorRange* range, uint32_t numDescriptors, uint32_t shaderRegister, RHI::ShaderStage stage, RHI::DescriptorType type);

	}
}


