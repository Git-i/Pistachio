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
#include <thread>
void ProcessIndices(const aiMesh* pMesh, std::vector<unsigned short>& indices)
{
	for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
	{
		const auto& face = pMesh->mFaces[i];
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}
}
namespace Pistachio {
	Mesh* Mesh::Create(const char* filepath)
	{
		Mesh* result = new Mesh;
		Assimp::Importer imp;
		const aiScene* pScene = imp.ReadFile(filepath, aiProcess_Triangulate|aiProcess_JoinIdenticalVertices);
		const aiMesh* pMesh = pScene->mMeshes[0];
		result->m_vertices.reserve(pMesh->mNumVertices);
		result->m_indices.reserve(pMesh->mNumFaces * 3);
		std::thread worker(ProcessIndices, (pMesh), std::ref(result->m_indices));
		for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
		{
			result->m_vertices.emplace_back(
				pMesh->mVertices[i].x, pMesh->mVertices[i].y, pMesh->mVertices[i].z ,
				pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z ,
				pMesh->mTextureCoords[0][i].x, pMesh->mTextureCoords[0][i].y
			);
		}
		worker.join();
		result->m_VertexBuffer.CreateStack(result->m_vertices.data(), sizeof(Vertex) * pMesh->mNumVertices, sizeof(Vertex));
		result->m_IndexBuffer.CreateStack(result->m_indices.data(), sizeof(unsigned short) * pMesh->mNumFaces * 3, sizeof(unsigned short));
		imp.FreeScene();
		return result;
	}
	void Mesh::CreateStack(const char* filepath)
	{
		Assimp::Importer imp;
		const aiScene* pScene = imp.ReadFile(filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices| aiProcess_FlipUVs);
		std::vector<const aiMesh*> pmeshes;
		pmeshes.reserve(pScene->mNumMeshes);
		int vertices = 0;
		int faces = 0;
		for (int i = 0; i < pScene->mNumMeshes; i++) {
			pmeshes.push_back(pScene->mMeshes[i]);
			vertices += pScene->mMeshes[i]->mNumVertices;
			faces += pScene->mMeshes[i]->mNumFaces;
		}
		m_vertices.reserve(vertices);
		m_indices.reserve(faces * 3);
		for (int i = 0; i < pScene->mNumMeshes; i++) {
			std::thread worker(ProcessIndices, (pmeshes[i]), std::ref(m_indices));
			for (unsigned int j = 0; j < pmeshes[i]->mNumVertices; j++)
			{
				m_vertices.emplace_back(
					pmeshes[i]->mVertices[j].x, pmeshes[i]->mVertices[j].y, pmeshes[i]->mVertices[j].z,
					pmeshes[i]->mNormals[j].x, pmeshes[i]->mNormals[j].y, pmeshes[i]->mNormals[j].z,
					pmeshes[i]->mTextureCoords[0][j].x, pmeshes[i]->mTextureCoords[0][j].y
				);
			}
			worker.join();
		}
		m_VertexBuffer.CreateStack(m_vertices.data(), sizeof(Vertex) * vertices, sizeof(Vertex));
		m_IndexBuffer.CreateStack(m_indices.data(), sizeof(unsigned short) * faces * 3, sizeof(unsigned short));
		imp.FreeScene();
	}
	BufferLayout* Mesh::GetLayout()
	{
		return layout;
	}
}