#include "FormatsAndTypes.h"
#include "Pistachio/Core/Error.h"
#include "ptpch.h"
#include "Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/Logger.hpp"
#include "assimp/DefaultLogger.hpp"
#include "../Core/Log.h"
#include "Renderer.h"
#include <thread>
Pistachio::BufferLayout Pistachio::Mesh::layout[] = {
			{"POSITION", RHI::Format::FLOAT3, 0},
			{"NORMAL",   RHI::Format::FLOAT3, 12},
			{"UV",       RHI::Format::FLOAT2, 24}
};
void ProcessIndices(const aiMesh* pMesh, std::vector<unsigned int>& indices)
{
	for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
	{
		const auto& face = pMesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; j++)
		indices.push_back(face.mIndices[j]);
	}
}
namespace Pistachio {
	Result<Mesh*> Mesh::Create(const char* filepath, std::uint32_t index)
	{
		PT_PROFILE_FUNCTION()
		Mesh* result = new Mesh;
		auto err = result->CreateStack(filepath, index);
		if(err.GetErrorType() != ErrorType::Success)
		{
			delete result;
			return ezr::err(std::move(err));
		}
		return ezr::ok(std::move(result));
	}
	Error Mesh::CreateStack(const char* filepath, std::uint32_t index)
	{
		PT_PROFILE_FUNCTION()
		m_vertices.clear();
		m_indices.clear();
		PT_CORE_INFO("Loading Mesh {0}", filepath);
		if(!Pistachio::Error::CheckFileExistence(filepath))
			return Error(ErrorType::NonExistentFile, std::string(__FUNCTION__) + ", filename: " + filepath);
		Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE, aiDefaultLogStream_STDOUT);
		Assimp::Importer imp;
		const aiScene* pScene = imp.ReadFile(filepath, aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded);
		
		Assimp::DefaultLogger::kill();
		const aiMesh* pmeshes = pScene->mMeshes[index];
		m_vertices.reserve(pmeshes->mNumVertices);
		m_indices.reserve(pmeshes->mNumFaces * 3);	
		std::thread worker(ProcessIndices, (pmeshes), std::ref(m_indices));
		for (unsigned int j = 0; j < pmeshes->mNumVertices; j++)
		{
			auto& vert = m_vertices.emplace_back();
			vert.position =	{pmeshes->mVertices[j].x, pmeshes->mVertices[j].y, pmeshes->mVertices[j].z};
			if(pmeshes->HasNormals())vert.normal={pmeshes->mNormals[j].x, pmeshes->mNormals[j].y, pmeshes->mNormals[j].z};
			if(pmeshes->HasTextureCoords(0))vert.TexCoord={pmeshes->mTextureCoords[0][j].x, pmeshes->mTextureCoords[0][j].y};
		}
		worker.join();
		m_VertexBuffer = Renderer::AllocateVertexBuffer(sizeof(Vertex) * pmeshes->mNumVertices, m_vertices.data());
		m_IndexBuffer = Renderer::AllocateIndexBuffer(sizeof(unsigned int) * pmeshes->mNumFaces * 3, m_indices.data());
		imp.FreeScene();
		return Error(Pistachio::ErrorType::Success, "NO ERROR");
	}
	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
	{
		PT_PROFILE_FUNCTION()
		m_vertices = vertices;
		m_indices = indices;
		m_VertexBuffer = Renderer::AllocateVertexBuffer((uint32_t)(sizeof(Vertex) * m_vertices.size()), m_vertices.data());
		m_IndexBuffer = Renderer::AllocateIndexBuffer((uint32_t)(sizeof(unsigned int) * m_indices.size()), m_indices.data());
	}
	Mesh::Mesh(const std::vector<Vertex>&& vertices, const std::vector<unsigned int>&& indices)
	{
		PT_PROFILE_FUNCTION()
		m_vertices = vertices;
		m_indices = indices;
		m_VertexBuffer = Renderer::AllocateVertexBuffer((uint32_t)(sizeof(Vertex) * m_vertices.size()), m_vertices.data());
		m_IndexBuffer = Renderer::AllocateIndexBuffer((uint32_t)(sizeof(unsigned int) * m_indices.size()), m_indices.data());
	}
	Mesh::~Mesh()
	{
		Renderer::FreeVertexBuffer(m_VertexBuffer);
		Renderer::FreeIndexBuffer(m_IndexBuffer);
	}
	BufferLayout* Mesh::GetLayout()
	{
		return layout;
	}
}