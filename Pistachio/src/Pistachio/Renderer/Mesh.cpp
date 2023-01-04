#include "ptpch.h"
#include "Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Logger.hpp"
#include "assimp/DefaultLogger.hpp"
#include "../Core/Log.h"
Pistachio::BufferLayout Pistachio::Mesh::layout[] = {
			{"POSITION", Pistachio::BufferLayoutFormat::FLOAT3, 0},
			{"NORMAL", Pistachio::BufferLayoutFormat::FLOAT3, 12},
			{"UV", Pistachio::BufferLayoutFormat::FLOAT2, 24}
};
#include <thread>
void ProcessIndices(const aiMesh* pMesh, std::vector<unsigned int>& indices)
{
	for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
	{
		const auto& face = pMesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; j++)
		indices[i*3+j] = (face.mIndices[j]);
	}
}
namespace Pistachio {
	Mesh* Mesh::Create(const char* filepath)
	{
		Mesh* result = new Mesh;
		result->CreateStack(filepath);
		return result;
	}
	Error Mesh::CreateStack(const char* filepath)
	{
		PT_CORE_INFO("Loading Mesh {0}", filepath);
		if(!Pistachio::Error::CheckFileExistence(filepath))
			return Error(ErrorType::NonExistentFile, std::string(__FUNCTION__) + ", filename: " + filepath);
		Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE, aiDefaultLogStream_STDOUT);
		Assimp::Importer imp;
		const aiScene* pScene = imp.ReadFile(filepath, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		
		Assimp::DefaultLogger::kill();
		const aiMesh* pmeshes = pScene->mMeshes[0];
		m_vertices.reserve(pmeshes->mNumVertices);
		m_indices.reserve(pmeshes->mNumFaces * 3);	
		//std::thread worker(ProcessIndices, (pmeshes), std::ref(m_indices));
		for (unsigned int j = 0; j < pmeshes->mNumVertices; j++)
		{
			m_vertices.push_back(Vertex(
				pmeshes->mVertices[j].x, pmeshes->mVertices[j].y, pmeshes->mVertices[j].z,
				pmeshes->mNormals[j].x, pmeshes->mNormals[j].y, pmeshes->mNormals[j].z,
				pmeshes->mTextureCoords[0][j].x, pmeshes->mTextureCoords[0][j].y
			));
		}
		for (unsigned int i = 0; i < pmeshes->mNumFaces; i++)
		{
			const auto& face = pmeshes->mFaces[i];
			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				m_indices.push_back(face.mIndices[j]);
		}
		//worker.join();
		m_VertexBuffer.CreateStack(m_vertices.data(), sizeof(Vertex) * pmeshes->mNumVertices, sizeof(Vertex));
		m_IndexBuffer.CreateStack(m_indices.data(), sizeof(unsigned int) * pmeshes->mNumFaces * 3, sizeof(unsigned int));
		imp.FreeScene();
		return Error(Pistachio::ErrorType::Success, "NO ERROR");
	}
	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
	{
		m_vertices = vertices;
		m_indices = indices;
		m_VertexBuffer.CreateStack(m_vertices.data(), sizeof(Vertex) * m_vertices.size(), sizeof(Vertex));
		//m_VertexBuffer.Bind();
		m_IndexBuffer.CreateStack(m_indices.data(), sizeof(unsigned int) * m_indices.size(), sizeof(unsigned int));
		//m_IndexBuffer.Bind();
	}
	BufferLayout* Mesh::GetLayout()
	{
		return layout;
	}
}