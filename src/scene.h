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
#include "lightManager.h"
#include "shadowManager.h"
class Scene {
public:
    Scene(const char* path);
    void addModel(Model&& model); // Accept Model by move
    void setCamera(const Camera& camera);
    Camera& getCamera();

    bool loadGLTF(const std::string& path);
    void draw(Shader& shader);
    void drawWithShadows(Shader& shader, Shader& shadowShader);
    void setSkybox(const std::string& directory);
    void setSkyboxShader(const std::string& vertexPath, const std::string& fragmentPath);

    LightManager& getLightManager() { return lightManager; }
    const LightManager& getLightManager() const { return lightManager; }

    ShadowManager& getShadowManager() { return shadowManager; }
    const ShadowManager& getShadowManager() const { return shadowManager; }

    std::vector<Model>& getModels() { return models; }
    const std::vector<Model>& getModels() const { return models; }

    size_t addDirectionalLight(const glm::vec3& direction, const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f);
    size_t addPointLight(const glm::vec3& position, const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f);
    size_t addSpotLight(const glm::vec3& position, const glm::vec3& direction, 
                       const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f,
                       float innerCutoff = 12.5f, float outerCutoff = 17.5f);

    void enableShadowsForLight(size_t lightIndex, unsigned int resolution = 2048);
    void disableShadowsForLight(size_t lightIndex);
    void setSceneBounds(const glm::vec3& center, float radius);

private:
    std::vector<Model> models;
    std::unique_ptr<Skybox> skybox;
    std::unique_ptr<Shader> skyboxShader;
    Camera camera;
    Model assimpMeshToModel(aiMesh* mesh, const aiScene* scene, const std::string& gltfFilePath);
    LightManager lightManager;
    ShadowManager shadowManager;
};

#endif // SCENE