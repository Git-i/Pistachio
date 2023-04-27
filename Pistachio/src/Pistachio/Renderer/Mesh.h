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
	struct Face
	{
		unsigned int i0, i1, i2;
	};
	class Mesh {
	public:
		Mesh(){}
		static Mesh* Create(const char* filepath);
		Error CreateStack(const char* filepath);
		Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
		static BufferLayout* GetLayout();
		inline static int GetLayoutSize() { return 3; }
		inline const VertexBuffer& GetVertexBuffer() const { return m_VertexBuffer; }
		inline const IndexBuffer& GetIndexBuffer() const { return m_IndexBuffer; }
		inline void DestroyMesh() { m_VertexBuffer.ShutDown(); m_IndexBuffer.ShutDown(); m_vertices.clear(); m_indices.clear();};
		~Mesh() { DestroyMesh(); };
	private:
		static BufferLayout layout[];
		std::vector<Vertex> m_vertices;
		std::vector<unsigned int> m_indices;
		VertexBuffer m_VertexBuffer;
		IndexBuffer m_IndexBuffer;
	};
}