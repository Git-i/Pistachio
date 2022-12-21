#pragma once

namespace Pistachio {
	class RenderTexture {
	public:
		void Bind();
		void CreateStack(int width, int height, int miplevels = 1);
		void Clear(float* clearcolor);
		static RenderTexture* Create(int width, int height, int miplevels = 1);
		ID3D11ShaderResourceView* shaderResourceView;
	private:
		ID3D11Texture2D* renderTargetTexture;
		ID3D11RenderTargetView* renderTargetView;
		ID3D11DepthStencilView* pDSV;
	};
	class RenderCubeMap {	
	public:
		void Bind(int slot);
		void CreateStack(int width, int height, int miplevels = 1);
		void Clear(float* clearcolor, int slot);
		static RenderCubeMap* Create(int width, int height, int miplevels = 1);
		ID3D11ShaderResourceView* shaderResourceView;
	private:
		ID3D11Texture2D* renderTargetTexture;
		ID3D11RenderTargetView* renderTargetView[6];
		ID3D11DepthStencilView* pDSV;
	};
}
