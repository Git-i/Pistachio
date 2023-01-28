#pragma once
#include "Buffer.h"
#include "Shader.h"
#include "../Core/Error.h"
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
		Vertex(){}
	};
	class Mesh {
	public:
		Mesh(){}
		static Mesh* Create(const char* filepath);
		Error CreateStack(const char* filepath);
		Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
		static BufferLayout* GetLayout();
		inline static int GetLayoutSize() { return 3; }
		inline VertexBuffer* GetVertexBuffer() { return &m_VertexBuffer; }
		inline IndexBuffer* GetIndexBuffer() { return &m_IndexBuffer; }
		inline void DestroyMesh() { m_VertexBuffer.ShutDown(); m_IndexBuffer.ShutDown(); };
		~Mesh() { m_VertexBuffer.ShutDown(); m_IndexBuffer.ShutDown(); };
	private:
		static BufferLayout layout[];
		std::vector<Vertex> m_vertices;
		std::vector<unsigned int> m_indices;
		VertexBuffer m_VertexBuffer;
		IndexBuffer m_IndexBuffer;
	};
}