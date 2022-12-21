#pragma once
#include "Buffer.h"
#include "Shader.h"

namespace Pistachio {
	struct Vertex
	{
		struct {
			float x, y, z;
		} position;
		struct {
			float x, y, z;
		} normal;
		struct
		{
			float u, v;
		} TexCoord;
		Vertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
			: position{ px, py, pz },normal{nx, ny, nz}, TexCoord{u, v}
		{
		}
	};
	class Mesh {
	public:
		static Mesh* Create(const char* filepath);
		void CreateStack(const char* filepath);
		static BufferLayout* GetLayout();
		inline static int GetLayoutSize() { return 3; }
		inline VertexBuffer* GetVertexBuffer() { return &m_VertexBuffer; }
		inline IndexBuffer* GetIndexBuffer() { return &m_IndexBuffer; }
	private:
		static BufferLayout layout[];
		std::vector<Vertex> m_vertices;
		std::vector<unsigned short> m_indices;
		VertexBuffer m_VertexBuffer;
		IndexBuffer m_IndexBuffer;
	};
}