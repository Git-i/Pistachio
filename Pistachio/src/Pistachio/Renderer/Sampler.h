#pragma once
#include "../Core.h"
#include "RendererID_t.h"
namespace Pistachio {
	enum class PISTACHIO_API TextureAddress {
		Border, Clamp, Mirror, Wrap
	};
	enum class PISTACHIO_API Filter {
		Point = 0, Linear = 1
	};
	enum class PISTACHIO_API ComparisonFunc
	{
		Never = 1,
		Less = 2,
		Equal = 3,
		LessOrEqual = 4,
		Greater = 5,
		NotEqual = 6,
		GreaterOrEqual = 7,
		Always = 8
	};
	struct PISTACHIO_API SamplerStateDesc
	{
		Filter magFilter;
		Filter minFilter;
		Filter mipFilter;
		TextureAddress AddressU;
		TextureAddress AddressV;
		TextureAddress AddressW;
		std::uint32_t MaxAnisotropy;
		ComparisonFunc func;
		float MipLODBias;
		float MinLOD;
		float MaxLOD;
		float BorderColor[4];
		bool ComparisonEnable;
		bool AnisotropyEnable;
		static const SamplerStateDesc Default;
	};
	class PISTACHIO_API SamplerState
	{
	public:
		static SamplerState* Create(const SamplerStateDesc& stateDesc);
		void Bind(int slot = 0);
		~SamplerState() = default;
	private:
		PlatformRendererID_t ImageSamplerState;
	};
}