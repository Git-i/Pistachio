#pragma once
#include "Buffer.h"
#include "Shader.h"
#include "../Core/Error.h"
namespace Pistachio {
	struct PISTACHIO_API Vertex
	{
		struct {
			float x, y, z;
		} position = {0,0,0};
		struct {
			float x, y, z;
		} normal = {0,0,0};
		struct
		{
			float u, v;
		} TexCoord = {0,0};
		Vertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
			: position{ px, py, pz },normal{nx, ny, nz}, TexCoord{u, v}
		{
		}
		Vertex(){}
	};
	struct PISTACHIO_API Face
	{
		unsigned int i0, i1, i2;
	};
	class PISTACHIO_API Mesh {
	public:
		Mesh(){}
		static Mesh* Create(const char* filepath, std::uint32_t index = 0);
		Error CreateStack(const char* filepath, std::uint32_t index = 0);
		Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
		static BufferLayout* GetLayout();
		inline static int GetLayoutSize() { return 3; }
		inline const VertexBuffer& GetVertexBuffer() const { return m_VertexBuffer; }
		inline const IndexBuffer& GetIndexBuffer() const { return m_IndexBuffer; }
	private:
		static BufferLayout layout[];
		std::vector<Vertex> m_vertices;
		std::vector<unsigned int> m_indices;
		VertexBuffer m_VertexBuffer;
		IndexBuffer m_IndexBuffer;
	};
}