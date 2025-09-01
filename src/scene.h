#ifndef SCENE
#define SCENE

#include "model.h"
#include <vector>
#include "camera.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "shader.h"
#include "skybox.h"

class Scene {
public:
    Scene(const char* path);
    void addModel(Model&& model); // Accept Model by move
    void setCamera(const Camera& camera);
    const std::vector<Model>& getModels() const;
    Camera& getCamera();

    bool loadGLTF(const std::string& path);
    void draw(Shader& shader);

    void setSkybox(const std::string& directory);
    void setSkyboxShader(const std::string& vertexPath, const std::string& fragmentPath);

private:
    std::vector<Model> models;
    std::unique_ptr<Skybox> skybox;
    std::unique_ptr<Shader> skyboxShader;
    Camera camera;
    Model assimpMeshToModel(aiMesh* mesh, const aiScene* scene, const std::string& gltfFilePath);
};

#endif // SCENE