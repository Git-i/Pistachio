#include "ptpch.h"
#include "Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
Pistachio::BufferLayout Pistachio::Mesh::layout[] = {
			{"POSITION", Pistachio::BufferLayoutFormat::FLOAT3, 0},
			{"NORMAL", Pistachio::BufferLayoutFormat::FLOAT3, 12},
			{"UV", Pistachio::BufferLayoutFormat::FLOAT2, 24}
};
namespace Pistachio {
	Mesh* Mesh::Create(const char* filepath)
	{
		Mesh* result = new Mesh;
		Assimp::Importer imp;
		const aiScene* pScene = imp.ReadFile(filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);
		const aiMesh* pMesh = pScene->mMeshes[0];
		result->m_vertices.reserve(pMesh->mNumVertices);
		result->m_indices.reserve(pMesh->mNumFaces * 3);
		for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
		{
			result->m_vertices.emplace_back(
				pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z ,
				pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z ,
				pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y
			);
		}
		for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
		{
			const auto& face = pMesh->mFaces[i];
			result->m_indices.push_back(face.mIndices[0]);
			result->m_indices.push_back(face.mIndices[1]);
			result->m_indices.push_back(face.mIndices[2]);
		}
		result->m_VertexBuffer = VertexBuffer::Create(result->m_vertices.data(), sizeof(Vertex) * pMesh->mNumVertices, sizeof(Vertex));
		result->m_IndexBuffer = IndexBuffer::Create(result->m_indices.data(), sizeof(unsigned short) * pMesh->mNumFaces * 3, sizeof(unsigned short));
		imp.FreeScene();
		return result;
	}
	void Mesh::CreateStack(const char* filepath)
	{
		Assimp::Importer imp;
		const aiScene* pScene = imp.ReadFile(filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices| aiProcess_FlipUVs);
		const aiMesh* pMesh = pScene->mMeshes[0];
		m_vertices.reserve(pMesh->mNumVertices);
		m_indices.reserve(pMesh->mNumFaces * 3);
		for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
		{
			m_vertices.emplace_back(
				pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z,
				pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z,
				pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y
			);
		}
		for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
		{
			const auto& face = pMesh->mFaces[i];
			m_indices.push_back(face.mIndices[0]);
			m_indices.push_back(face.mIndices[1]);
			m_indices.push_back(face.mIndices[2]);
		}
		m_VertexBuffer = VertexBuffer::Create(m_vertices.data(), sizeof(Vertex) * pMesh->mNumVertices, sizeof(Vertex));
		m_IndexBuffer = IndexBuffer::Create(m_indices.data(), sizeof(unsigned short) * pMesh->mNumFaces * 3, sizeof(unsigned short));
		imp.FreeScene();
	}
	BufferLayout* Mesh::GetLayout()
	{
		return layout;
	}
}