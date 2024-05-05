#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

namespace Pistachio {
    class Model
    {
    public:
        // model data 
        std::vector<Mesh> meshes;
        ~Model() { for (auto& mesh : meshes) mesh.DestroyMesh(); }
        // constructor, expects a filepath to a 3D model.
        Model(const char* path)
        {
            loadModel(path);
        }
    private:
        // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
        Error loadModel(const char* path);
        // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    };
}