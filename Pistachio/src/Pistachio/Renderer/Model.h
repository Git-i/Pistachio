#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "../Asset/RefCountedObject.h"
#include "Pistachio\Core\Math.h"
namespace Pistachio {
    class Model : public RefCountedObject
    {
    public:
        // model data 
        std::vector<Mesh> meshes;
        std::vector<BoundingBox> aabbs;
        // constructor, expects a filepath to a 3D model.
        Model(const char* path)
        {
            loadModel(path);
        }

        static Model* Create(const char* path);
    private:
        // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
        Error loadModel(const char* path);
        // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
        void processNode(aiNode* node, const aiScene* scene);
        void processMesh(aiMesh* mesh, const aiScene* scene);
    };
}