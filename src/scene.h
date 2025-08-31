#ifndef SCENE
#define SCENE

#include "model.h"
#include <vector>
#include "camera.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "shader.h"

class Scene {
public:
    Scene(const char* path);
    void addModel(Model&& model); // Accept Model by move
    void setCamera(const Camera& camera);
    const std::vector<Model>& getModels() const;
    const Camera& getCamera() const;
    bool loadGLTF(const std::string& path);
    void draw(Shader& shader);

private:
    std::vector<Model> models;
    Camera camera;
    Model assimpMeshToModel(aiMesh* mesh, const aiScene* scene, const std::string& gltfFilePath);
};

#endif // SCENE