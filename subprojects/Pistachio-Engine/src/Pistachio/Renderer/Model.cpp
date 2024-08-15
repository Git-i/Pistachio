#include "ptpch.h"
#include "Model.h"
#include "assimp/Logger.hpp"
#include "../Core/Log.h"
#include "assimp/DefaultLogger.hpp"
#include "../Core/Error.h"
/*
 * Most of the code here is from https://www.learnopengl.com
 * It's not that i can't write it, i just didnt feel like
 * Ill probably get to optimize/potentially rewrite it later on
*/
namespace Pistachio {
    Result<Model*> Model::Create(const char* path)
    {
        Model* md = new Model;
        auto e = md->loadModel(path);
        if(e.GetErrorType() != ErrorType::Success)
        {
            delete md;
            return ezr::err(std::move(e));
        }
        else
        {
            return ezr::ok(std::move(md));
        }
    }
    Error Model::loadModel(const char* path)
    {
        PT_PROFILE_FUNCTION();
        PT_CORE_INFO("Loading Model {0}", path);
        if (!Pistachio::Error::CheckFileExistence(path))
            return Error(ErrorType::NonExistentFile, std::string(__FUNCTION__) + ", filename: " + path);
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ConvertToLeftHanded | aiProcess_GenBoundingBoxes);
        if (!scene)
        {
            PT_CORE_ERROR(importer.GetErrorString());
            return Error(ErrorType::Unknown, std::string(__FUNCTION__));
        }
        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
        return Error(ErrorType::Success, std::string(__FUNCTION__));
    }
    void Model::processNode(aiNode* node, const aiScene* scene)
    {
        PT_PROFILE_FUNCTION();
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh, scene);
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }
    void Model::processMesh(aiMesh* mesh, const aiScene* scene)
    {
        PT_PROFILE_FUNCTION();
        // data to fill
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        
        aiVector3D boundingBoxCentre = (mesh->mAABB.mMin + mesh->mAABB.mMax) * .5f;
        aiVector3D boundingBoxExtents = (mesh->mAABB.mMax - mesh->mAABB.mMin) * .5f;
        Vector3 bbCentre = { boundingBoxCentre.x , boundingBoxCentre.y, boundingBoxCentre.z };
        Vector3 bbExtents = { boundingBoxExtents.x, boundingBoxExtents.y, boundingBoxExtents.z };
        aabbs.emplace_back(bbCentre, bbExtents);

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            // positions
            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;
            // normals
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
            // texture coordinates
            if (mesh->mTextureCoords[0])
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
        meshes.emplace_back(std::move(vertices), std::move(indices));
    }
}