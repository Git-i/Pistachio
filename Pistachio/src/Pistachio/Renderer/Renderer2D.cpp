#include "ptpch.h"
#include "Renderer2D.h"
#include "Buffer.h"
#include "Shader.h"
#include "RendererBase.h"
namespace Pistachio {
	struct Renderer2DData {
		Ref<Shader> FlatColorShader;
		Ref<VertexBuffer> quadVB;
		Ref<IndexBuffer> quadIB;
	};

	static Renderer2DData* s_Data;

	void Renderer2D::Init()
	{
		s_Data = new Renderer2DData();
		float vertices[4 * 3] = {
			-1.f, -1.f, 0.f,
			 1.f, -1.f, 0.f,
			 1.f,  1.f, 1.f,
			-1.f,  1.f, 0.f
		};
		unsigned short indices[6] = {
			2,1,0,0,3,2
		};
		s_Data->quadVB.reset(Pistachio::VertexBuffer::Create(&vertices, sizeof(float) * 12, 12));
		s_Data->quadIB.reset(Pistachio::IndexBuffer::Create(&indices, 6 * sizeof(unsigned short), sizeof(unsigned short)));
		s_Data->FlatColorShader = std::make_shared<Shader>(L"2D_vs.cso", L"FlatColorShader.cso");
		BufferLayout layout[1] = { {"POSITION", BufferLayoutFormat::FLOAT3, 0} };
		s_Data->FlatColorShader->CreateLayout(layout, 1);
		
	}

	void Renderer2D::Shutdown()
	{
		s_Data->quadVB->ShutDown();
		s_Data->quadIB->ShutDown();
		s_Data->FlatColorShader->Shutdown();
		delete s_Data;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		s_Data->FlatColorShader->Bind(ShaderType::Vertex);
		s_Data->FlatColorShader->Bind(ShaderType::Pixel);
		auto viewproj = camera.GetViewProjectionMatrix();
		s_Data->FlatColorShader->SetVSRandomBuffer(&viewproj, sizeof(DirectX::XMMATRIX), 0);
	}

	void Renderer2D::EndScene()
	{
	}

	void Renderer2D::DrawQuad(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& size, const DirectX::XMVECTOR& color)
	{
		s_Data->FlatColorShader->Bind(ShaderType::Vertex);
		s_Data->FlatColorShader->Bind(ShaderType::Pixel);
		DirectX::XMMATRIX transform = DirectX::XMMatrixTranspose(DirectX::XMMatrixTransformation2D(DirectX::XMVectorZero(), 0, DirectX::XMLoadFloat2(&size), DirectX::XMVectorZero(), 0.f, DirectX::XMLoadFloat3(&position)));
		s_Data->FlatColorShader->SetVSRandomBuffer(&transform, sizeof(transform), 1);
		Buffer buffer = { s_Data->quadVB.get(), s_Data->quadIB.get() };
		s_Data->FlatColorShader->SetPSBuffer(&color, sizeof(DirectX::XMVECTOR), 0);
		RendererBase::DrawIndexed(buffer);
	}
	void Renderer2D::DrawQuad(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& size, const Texture& texture, const DirectX::XMVECTOR& color)
	{
		s_Data->FlatColorShader->Bind(ShaderType::Vertex);
		s_Data->FlatColorShader->Bind(ShaderType::Pixel);
		DirectX::XMMATRIX transform = DirectX::XMMatrixTranspose(DirectX::XMMatrixTransformation2D(DirectX::XMVectorZero(), 0, DirectX::XMLoadFloat2(&size), DirectX::XMVectorZero(), 0.f, DirectX::XMLoadFloat3(&position)));
		s_Data->FlatColorShader->SetVSRandomBuffer(&transform, sizeof(transform), 1);
		Buffer buffer = { s_Data->quadVB.get(), s_Data->quadIB.get() };
		s_Data->FlatColorShader->SetPSBuffer(&color, sizeof(DirectX::XMVECTOR), 0);
		texture.Bind(4);
		RendererBase::DrawIndexed(buffer);
	}
}