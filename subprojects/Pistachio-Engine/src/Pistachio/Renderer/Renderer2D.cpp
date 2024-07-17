#include "ptpch.h"
#include "Renderer2D.h"
#include "Buffer.h"
#include "Shader.h"
#include "RendererBase.h"
#include "Renderer.h"
namespace Pistachio {
	struct QuadVertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 TexCoord;
		DirectX::XMFLOAT4 color;
		float textureindex;
		// Editor-only
		int EntityID = 0;
	};
	struct Renderer2DData {
		const unsigned int MaxQuads =  20000;
		const unsigned int MaxVertices = MaxQuads * 4;
		const unsigned int MaxIndices =  MaxQuads * 6;
		Ref<Shader> _2DShader;
		Ref<VertexBuffer> quadVB;
		Ref<IndexBuffer> quadIB;
		Ref<Texture2D> whiteTexture;

		unsigned int QuadIndexCount = 0;
		QuadVertex* QuadVerticesBase = nullptr;
		QuadVertex* QuadVerticesPtr = nullptr;

		std::array<Ref<Texture2D>, 32> TextureSlots;
		unsigned int TextureSlotsIndex = 1;

		DirectX::XMFLOAT3 positions[4] = {
			{ -1, -1, 0  },
			{  1, -1, 0  },
			{  1,  1, 0  },
			{ -1,  1, 0  }
		};

	};

	static Renderer2DData s_Data;

	void Renderer2D::Init()
	{
		PT_PROFILE_FUNCTION();
		std::cout << "renderer2d" << std::endl;
		unsigned int* quadIndices = new unsigned int[s_Data.MaxIndices];
		unsigned int offset = 0;
		for (unsigned int i = 0; i < s_Data.MaxIndices; i+=6)
		{
			quadIndices[i + 0] = offset + 2;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 0;
		
			quadIndices[i + 3] = offset + 0;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 2;
		
			offset += 4;
		}
		s_Data.QuadVerticesBase = new QuadVertex[s_Data.MaxVertices];
		s_Data.quadVB.reset(Pistachio::VertexBuffer::Create(nullptr, sizeof(QuadVertex) * s_Data.MaxVertices, sizeof(QuadVertex)).value());
		s_Data.quadIB.reset(Pistachio::IndexBuffer::Create(quadIndices, s_Data.MaxIndices * sizeof(unsigned int), sizeof(unsigned int)).value());
		delete[] quadIndices;
		unsigned char white[4] = { 255, 255, 255, 255 };
		//s_Data.whiteTexture.reset(Pistachio::Texture2D::Create(1, 1, TextureFormat::RGBA8U,white));
		//s_Data._2DShader = std::make_shared<Shader>(L"resources/shaders/vertex/2D_vs.cso", L"resources/shaders/pixel/2D_fs.cso");
		BufferLayout layout[5] = { {"POSITION", BufferLayoutFormat::FLOAT3, 0  }, 
									{"UV",      BufferLayoutFormat::FLOAT2, 12 }, 
									{"COLOR",   BufferLayoutFormat::FLOAT4, 20 }, 
									{"TEXINDEX",BufferLayoutFormat::FLOAT,  36 },
									{"ID", BufferLayoutFormat::INT, 40}
								 };
		s_Data.TextureSlots[0] = s_Data.whiteTexture;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		//s_Data._2DShader->Bind();
		auto viewproj = camera.GetViewProjectionMatrix();
		
		s_Data.QuadVerticesPtr = s_Data.QuadVerticesBase;
		s_Data.QuadIndexCount = 0;
		s_Data.TextureSlotsIndex = 1;
	}

	void Renderer2D::BeginScene(const RuntimeCamera& camera, const DirectX::XMMATRIX& transform)
	{
		//s_Data._2DShader->Bind();
		auto viewproj = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, transform) * DirectX::XMMATRIX(camera.GetProjection()));
		
		s_Data.QuadVerticesPtr = s_Data.QuadVerticesBase;
		s_Data.QuadIndexCount = 0;
		s_Data.TextureSlotsIndex = 1;
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		PT_PROFILE_FUNCTION()
		//s_Data._2DShader->Bind();
		auto viewproj = DirectX::XMMatrixTranspose(camera.GetViewProjection());
		
		s_Data.QuadVerticesPtr = s_Data.QuadVerticesBase;
		s_Data.QuadIndexCount = 0;
		s_Data.TextureSlotsIndex = 1;
	}

	void Renderer2D::EndScene()
	{
		PT_PROFILE_FUNCTION();
		uint32_t size = (uint32_t)((unsigned char*)s_Data.QuadVerticesPtr - (unsigned char*)s_Data.QuadVerticesBase);
		if (size) {
			s_Data.quadVB->SetData(s_Data.QuadVerticesBase, size);
			Flush();
		}
	}
	void Renderer2D::Flush()
	{
		for (unsigned int i = 0; i < s_Data.TextureSlotsIndex; i++)
		{
			s_Data.TextureSlots[i]->Bind(i);
		}
		//RendererBase::DrawIndexed({ s_Data.quadVB.get(), s_Data.quadIB.get() }, s_Data.QuadIndexCount);
	}

	void Renderer2D::FlushAndReset()
	{
		EndScene();
		s_Data.QuadVerticesPtr = s_Data.QuadVerticesBase;
		s_Data.QuadIndexCount = 0;
		s_Data.TextureSlotsIndex = 1;
	}

	void Renderer2D::DrawQuad(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color)
	{
		PT_PROFILE_FUNCTION()
			if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
				FlushAndReset();
		DirectX::XMMATRIX transform = DirectX::XMMatrixScaling(size.x, size.y, 1);
		transform *= DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&position));
		DirectX::XMFLOAT3 positions[4];
		for (int i = 0; i < 4; i++)
		{
			DirectX::XMStoreFloat3(&positions[i], DirectX::XMVector2Transform(DirectX::XMLoadFloat3(&s_Data.positions[i]), transform));
		}
		s_Data.QuadVerticesPtr->position = positions[0];
		s_Data.QuadVerticesPtr->TexCoord = { 0.f,0.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[1];
		s_Data.QuadVerticesPtr->TexCoord = { 1.f,0.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[2];
		s_Data.QuadVerticesPtr->TexCoord = { 1.f,1.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[3];
		s_Data.QuadVerticesPtr->TexCoord = { 0.f,1.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadIndexCount += 6;
	}
	void Renderer2D::DrawSprite(const DirectX::XMMATRIX& transform, const SpriteRendererComponent& src, int ID)
	{
		DirectX::XMFLOAT3 positions[4];
		for (int i = 0; i < 4; i++)
		{
			DirectX::XMStoreFloat3(&positions[i], DirectX::XMVector2Transform(DirectX::XMLoadFloat3(&s_Data.positions[i]), transform));
		}
		s_Data.QuadVerticesPtr->position = positions[0];
		s_Data.QuadVerticesPtr->TexCoord = { 0.f,0.f };
		s_Data.QuadVerticesPtr->color = src.Color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr->EntityID = ID;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[1];
		s_Data.QuadVerticesPtr->TexCoord = { 1.f,0.f };
		s_Data.QuadVerticesPtr->color = src.Color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr->EntityID = ID;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[2];
		s_Data.QuadVerticesPtr->TexCoord = { 1.f,1.f };
		s_Data.QuadVerticesPtr->color = src.Color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr->EntityID = ID;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[3];
		s_Data.QuadVerticesPtr->TexCoord = { 0.f,1.f };
		s_Data.QuadVerticesPtr->color = src.Color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr->EntityID = ID;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadIndexCount += 6;
	}
	void Renderer2D::DrawQuad(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& size, const Ref<Texture2D>& texture, const DirectX::XMFLOAT4& color)
	{
		PT_PROFILE_FUNCTION()
			if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
				FlushAndReset();
		float textureindex = 0.f;
		for (unsigned int i = 1; i < s_Data.TextureSlotsIndex; i++)
		{
			if(*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureindex = (float)i;
				break;
			}
		}

		if (textureindex == 0)
		{
			textureindex = (float)s_Data.TextureSlotsIndex;
			s_Data.TextureSlots[s_Data.TextureSlotsIndex] = texture;
			s_Data.TextureSlotsIndex++;
		}

		DirectX::XMMATRIX transform = DirectX::XMMatrixScaling(size.x, size.y, 1);
		transform *= DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&position));
		DirectX::XMFLOAT3 positions[4];
		for (int i = 0; i < 4; i++)
		{
			DirectX::XMStoreFloat3(&positions[i], DirectX::XMVector2Transform(DirectX::XMLoadFloat3(&s_Data.positions[i]), transform));
		}

		s_Data.QuadVerticesPtr->position = positions[0];
		s_Data.QuadVerticesPtr->TexCoord = { 0.f,0.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = textureindex;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[1];
		s_Data.QuadVerticesPtr->TexCoord = { 1.f,0.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = textureindex;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[2];
		s_Data.QuadVerticesPtr->TexCoord = { 1.f,1.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = textureindex;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[3];
		s_Data.QuadVerticesPtr->TexCoord = { 0.f,1.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = textureindex;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadIndexCount += 6;

	}
	void Renderer2D::DrawQuad(const DirectX::XMMATRIX& transform, const DirectX::XMFLOAT4& color)
	{
		DirectX::XMFLOAT3 positions[4];
		for (int i = 0; i < 4; i++)
		{
			DirectX::XMStoreFloat3(&positions[i], DirectX::XMVector2Transform(DirectX::XMLoadFloat3(&s_Data.positions[i]), transform));
		}
		s_Data.QuadVerticesPtr->position = positions[0];
		s_Data.QuadVerticesPtr->TexCoord = { 0.f,0.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[1];
		s_Data.QuadVerticesPtr->TexCoord = { 1.f,0.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[2];
		s_Data.QuadVerticesPtr->TexCoord = { 1.f,1.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[3];
		s_Data.QuadVerticesPtr->TexCoord = { 0.f,1.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadIndexCount += 6;
	}
	void Renderer2D::DrawQuad(const DirectX::XMMATRIX& transform, const Ref<Texture2D>& texture, const DirectX::XMFLOAT4& color)
	{
		DirectX::XMFLOAT3 positions[4];
		for (int i = 0; i < 4; i++)
		{
			DirectX::XMStoreFloat3(&positions[i], DirectX::XMVector2Transform(DirectX::XMLoadFloat3(&s_Data.positions[i]), transform));
		}
		s_Data.QuadVerticesPtr->position = positions[0];
		s_Data.QuadVerticesPtr->TexCoord = { 0.f,0.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[1];
		s_Data.QuadVerticesPtr->TexCoord = { 1.f,0.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[2];
		s_Data.QuadVerticesPtr->TexCoord = { 1.f,1.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[3];
		s_Data.QuadVerticesPtr->TexCoord = { 0.f,1.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadIndexCount += 6;
	}
	void Renderer2D::DrawRotatedQuad(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& size, const DirectX::XMFLOAT4& color, float angle)
	{
		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();
		PT_PROFILE_FUNCTION()
		DirectX::XMMATRIX transform = DirectX::XMMatrixScaling(size.x, size.y, 1);
		transform *= DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(angle));
		transform *= DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&position));
		DirectX::XMFLOAT3 positions[4];
		for (int i = 0; i < 4; i++)
		{
			DirectX::XMStoreFloat3(&positions[i], DirectX::XMVector2Transform(DirectX::XMLoadFloat3(&s_Data.positions[i]), transform));
		}
		s_Data.QuadVerticesPtr->position = positions[0];
		s_Data.QuadVerticesPtr->TexCoord = { 0.f,0.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[1];
		s_Data.QuadVerticesPtr->TexCoord = { 1.f,0.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[2];
		s_Data.QuadVerticesPtr->TexCoord = { 1.f,1.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadVerticesPtr->position = positions[3];
		s_Data.QuadVerticesPtr->TexCoord = { 0.f,1.f };
		s_Data.QuadVerticesPtr->color = color;
		s_Data.QuadVerticesPtr->textureindex = 0;
		s_Data.QuadVerticesPtr++;

		s_Data.QuadIndexCount += 6;
	}
	void Renderer2D::DrawRotatedQuad(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& size, const Ref<Texture2D>& texture, const DirectX::XMFLOAT4& color)
	{
	}
}