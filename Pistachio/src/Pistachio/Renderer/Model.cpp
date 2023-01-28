#include "ptpch.h"
#include "Model.h"
#include "assimp/Logger.hpp"
#include "../Core/Log.h"
#include "assimp/DefaultLogger.hpp"
#include "../Core/Error.h"
namespace Pistachio {
    Error Model::loadModel(const char* path)
    {
        PT_PROFILE_FUNCTION()
        PT_CORE_INFO("Loading Model {0}", path);
        if (!Pistachio::Error::CheckFileExistence(path))
            return Error(ErrorType::NonExistentFile, std::string(__FUNCTION__) + ", filename: " + path);
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded);
        if (!scene)
        {
            PT_CORE_ERROR(importer.GetErrorString());
        }
        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
        return Error(ErrorType::Success, std::string(__FUNCTION__));
    }
    void Model::processNode(aiNode* node, const aiScene* scene)
    {
        PT_PROFILE_FUNCTION()
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }
    Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
    {
        PT_PROFILE_FUNCTION()
        // data to fill
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            // positions
            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;
            // normals
            if (mesh->HasNormals())
            {
                vertex.normal.x = mesh->mNormals[i].x;
                vertex.normal.y = mesh->mNormals[i].y;
                vertex.normal.z = mesh->mNormals[i].z;
            }
            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vertex.TexCoord.u = mesh->mTextureCoords[0][i].x;
                vertex.TexCoord.v = mesh->mTextureCoords[0][i].y;
            }
            else
                vertex.TexCoord = { 0.0f, 0.0f };

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }
        return Mesh(vertices, indices);
    }
}