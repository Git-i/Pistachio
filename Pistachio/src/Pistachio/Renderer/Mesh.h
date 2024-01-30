#pragma once
#include <stdfloat>
#include "Buffer.h"
#include "Shader.h"
#include "../Core/Error.h"
#include "BufferHandles.h"
namespace Pistachio {
	//ptobably have different Vertex structs
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
		uint32_t i0, i1, i2;
	};
	class PISTACHIO_API Mesh {
	public:
		Mesh(){}
		static Mesh* Create(const char* filepath, std::uint32_t index = 0);
		Error CreateStack(const char* filepath, std::uint32_t index = 0);
		//Don't use the copy constructor, no reason to, keeping it for backward compatibility
		Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
		Mesh(const std::vector<Vertex>&& vertices, const std::vector<unsigned int>&& indices);
		~Mesh();
		static BufferLayout* GetLayout();
		inline static int GetLayoutSize() { return 3; }
		inline const RendererVBHandle GetVBHandle() const { return m_VertexBuffer; }
		inline const RendererIBHandle GetIBHandle() const { return m_IndexBuffer; }
	private:
		static BufferLayout layout[];
		std::vector<Vertex> m_vertices;
		std::vector<unsigned int> m_indices;
		RendererVBHandle m_VertexBuffer;
		RendererIBHandle m_IndexBuffer;
	};
}