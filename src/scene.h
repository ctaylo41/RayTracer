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

class Scene {
public:
    Scene(const char* path);
    void addModel(Model&& model); // Accept Model by move
    void setCamera(const Camera& camera);
    const std::vector<Model>& getModels() const;
    Camera& getCamera();

    bool loadGLTF(const std::string& path);
    void draw(Shader& shader);
    
    // New shadow-aware drawing methods
    void drawWithShadows(Shader& shader, const glm::mat4& lightSpaceMatrix, GLuint shadowMapTexture, bool shadowsEnabled);
    void drawDepthOnly(Shader& depthShader, const glm::mat4& lightSpaceMatrix);

    void setSkybox(const std::string& directory);
    void setSkyboxShader(const std::string& vertexPath, const std::string& fragmentPath);
    
    // Add getter methods for shadow system
    Skybox* getSkybox() const { return skybox.get(); }
    Shader* getSkyboxShader() const { return skyboxShader.get(); }

    LightManager& getLightManager() { return lightManager; }
    const LightManager& getLightManager() const { return lightManager; }

    size_t addDirectionalLight(const glm::vec3& direction, const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f);
    size_t addPointLight(const glm::vec3& position, const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f);
    size_t addSpotLight(const glm::vec3& position, const glm::vec3& direction, 
                       const glm::vec3& color = glm::vec3(1.0f), float intensity = 1.0f,
                       float innerCutoff = 12.5f, float outerCutoff = 17.5f);

    std::vector<Model>& getModels()  { return models; }
private:
    std::vector<Model> models;
    std::unique_ptr<Skybox> skybox;
    std::unique_ptr<Shader> skyboxShader;
    Camera camera;
    Model assimpMeshToModel(aiMesh* mesh, const aiScene* scene, const std::string& gltfFilePath);
    LightManager lightManager;
};

#endif // SCENE