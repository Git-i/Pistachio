#pragma once

namespace Pistachio {
	class Transform {
	public:
		DirectX::XMFLOAT3 translation = {0.0f, 0.0f, 0.0f};
		DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
		DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
		inline DirectX::XMMATRIX GetTransform() {
			transform = DirectX::XMMatrixTransformation(DirectX::XMVectorZero(), DirectX::XMVectorZero(), DirectX::XMLoadFloat3(&scale), DirectX::XMVectorZero(), DirectX::XMVectorZero(), DirectX::XMLoadFloat3(&translation));

			return transform; 
		}
	private:
		DirectX::XMMATRIX transform;
	};
}