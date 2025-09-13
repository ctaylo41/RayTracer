#include "scene.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <string>
#include <filesystem>



Scene::Scene(const char* path) {
    loadGLTF(path);
}

bool Scene::loadGLTF(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | 
    aiProcess_FlipUVs | 
    aiProcess_GenNormals |
    aiProcess_JoinIdenticalVertices |
    aiProcess_ValidateDataStructure |
    aiProcess_ImproveCacheLocality | 
    aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return false;
    }
    
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];
        Model model = assimpMeshToModel(mesh, scene, path);
        addModel(std::move(model)); // Use move semantics
    }

    // Load camera from glTF if present
    if (scene->mNumCameras > 0) {
        aiCamera* ai_cam = scene->mCameras[0];
        glm::vec3 position(ai_cam->mPosition.x, ai_cam->mPosition.y, ai_cam->mPosition.z);
        glm::vec3 up(ai_cam->mUp.x, ai_cam->mUp.y, ai_cam->mUp.z);
        // Calculate yaw and pitch from lookAt vector
        glm::vec3 lookAt(ai_cam->mLookAt.x, ai_cam->mLookAt.y, ai_cam->mLookAt.z);
        glm::vec3 front = glm::normalize(lookAt);
        float yaw = glm::degrees(atan2(front.z, front.x)) - 90.0f;
        float pitch = glm::degrees(asin(front.y));
        float fov = glm::degrees(ai_cam->mHorizontalFOV); // Assimp stores FOV in radians
        float nearPlane = ai_cam->mClipPlaneNear;
        float farPlane = ai_cam->mClipPlaneFar;

        unsigned int width = 1200;
        unsigned int height = 800; 

        Camera cam(position, up, yaw, pitch, fov, farPlane, nearPlane, width, height);
        std::cout << "Camera loaded from glTF: Position(" << position.x << ", " << position.y << ", " << position.z << "), Yaw: " << yaw << ", Pitch: " << pitch << ", FOV: " << fov << std::endl;
        setCamera(cam);
    } else {
        std::cout << "No camera found in glTF file." << std::endl;
        // Set a default camera
        glm::vec3 position(0.0f, 5.0f, 15.0f);
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        float yaw = -90.0f;
        float pitch = -15.0f;
        float fov = 45.0f;
        float nearPlane = 0.1f;
        float farPlane = 100.0f;
        unsigned int width = 1200;
        unsigned int height = 800;

        Camera cam(position, up, yaw, pitch, fov, farPlane, nearPlane, width, height);
        setCamera(cam);
    }

    calculatedSceneCenter = (loadingBounds.min + loadingBounds.max) * 0.5f;
    glm::vec3 extent = loadingBounds.max - loadingBounds.min;
    calculatedSceneRadius = glm::length(extent) * 0.5f;
    sceneBoundsCalculated = true;
    
    std::cout << "=== Scene Bounds from glTF ===" << std::endl;
    std::cout << "Min: (" << loadingBounds.min.x << ", " << loadingBounds.min.y << ", " << loadingBounds.min.z << ")" << std::endl;
    std::cout << "Max: (" << loadingBounds.max.x << ", " << loadingBounds.max.y << ", " << loadingBounds.max.z << ")" << std::endl;
    std::cout << "Center: (" << calculatedSceneCenter.x << ", " << calculatedSceneCenter.y << ", " << calculatedSceneCenter.z << ")" << std::endl;
    std::cout << "Radius: " << calculatedSceneRadius << std::endl;


    return true;
}

Model Scene::assimpMeshToModel(aiMesh* mesh, const aiScene* scene, const std::string& gltfFilePath) {
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> colors;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    // Vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        glm::vec3 vertex;
        vertex.x = mesh->mVertices[i].x;
        vertex.y = mesh->mVertices[i].y;
        vertex.z = mesh->mVertices[i].z;
        vertices.push_back(vertex);

        loadingBounds.min.x = std::min(loadingBounds.min.x, vertex.x);
        loadingBounds.min.y = std::min(loadingBounds.min.y, vertex.y);
        loadingBounds.min.z = std::min(loadingBounds.min.z, vertex.z);
        
        loadingBounds.max.x = std::max(loadingBounds.max.x, vertex.x);
        loadingBounds.max.y = std::max(loadingBounds.max.y, vertex.y);
        loadingBounds.max.z = std::max(loadingBounds.max.z, vertex.z);


    }

    // Indices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    // Normals
    if (mesh->HasNormals()) {
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            glm::vec3 normal;
            normal.x = mesh->mNormals[i].x;
            normal.y = mesh->mNormals[i].y;
            normal.z = mesh->mNormals[i].z;
            normals.push_back(normal);
        }
    }

    // Texture Coordinates
    if (mesh->HasTextureCoords(0)) {
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            glm::vec2 uv;
            uv.x = mesh->mTextureCoords[0][i].x;
            uv.y = mesh->mTextureCoords[0][i].y;
            uvs.push_back(uv);
        }
    }

    // Colors
    if (mesh->HasVertexColors(0)) {
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            glm::vec3 color;
            color.r = mesh->mColors[0][i].r;
            color.g = mesh->mColors[0][i].g;
            color.b = mesh->mColors[0][i].b;
            colors.push_back(color);
        }
    }

    // Texture References
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        
        
        // Get base color texture (albedo/diffuse)
        aiString baseColorPath;
        if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &baseColorPath) == AI_SUCCESS ||
            material->GetTexture(aiTextureType_DIFFUSE, 0, &baseColorPath) == AI_SUCCESS) {
            
            std::filesystem::path gltfDir = std::filesystem::path(gltfFilePath).parent_path();
            std::string texPath = baseColorPath.C_Str();
            
            if (!std::filesystem::path(texPath).is_absolute()) {
                texPath = (gltfDir / texPath).string();
            }
            
            Texture texture(texPath.c_str(), TextureType::Diffuse, 0);
            textures.push_back(std::move(texture));
        }
        
        // Get normal texture
        aiString normalPath;
        if (material->GetTexture(aiTextureType_NORMALS, 0, &normalPath) == AI_SUCCESS ||
            material->GetTexture(aiTextureType_HEIGHT, 0, &normalPath) == AI_SUCCESS) {
            
            std::filesystem::path gltfDir = std::filesystem::path(gltfFilePath).parent_path();
            std::string texPath = normalPath.C_Str();
            
            if (!std::filesystem::path(texPath).is_absolute()) {
                texPath = (gltfDir / texPath).string();
            }
            
            Texture texture(texPath.c_str(), TextureType::Normal, 1);
            textures.push_back(std::move(texture));
        }
        
        // Get metallic-roughness texture
        aiString metallicRoughnessPath;
        if (material->GetTexture(aiTextureType_METALNESS, 0, &metallicRoughnessPath) == AI_SUCCESS ||
            material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &metallicRoughnessPath) == AI_SUCCESS) {
            
            std::filesystem::path gltfDir = std::filesystem::path(gltfFilePath).parent_path();
            std::string texPath = metallicRoughnessPath.C_Str();
            
            if (!std::filesystem::path(texPath).is_absolute()) {
                texPath = (gltfDir / texPath).string();
            }
            
            Texture texture(texPath.c_str(), TextureType::Metallic, 2);
            textures.push_back(std::move(texture));
        }
        
        // Get occlusion texture
        aiString occlusionPath;
        if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &occlusionPath) == AI_SUCCESS ||
            material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &occlusionPath) == AI_SUCCESS) {
            
            std::filesystem::path gltfDir = std::filesystem::path(gltfFilePath).parent_path();
            std::string texPath = occlusionPath.C_Str();
            
            if (!std::filesystem::path(texPath).is_absolute()) {
                texPath = (gltfDir / texPath).string();
            }
            
            Texture texture(texPath.c_str(), TextureType::Occlusion, 3);
            textures.push_back(std::move(texture));
        }
        
        // Get emissive texture
        aiString emissivePath;
        if (material->GetTexture(aiTextureType_EMISSIVE, 0, &emissivePath) == AI_SUCCESS) {
            std::filesystem::path gltfDir = std::filesystem::path(gltfFilePath).parent_path();
            std::string texPath = emissivePath.C_Str();
            
            if (!std::filesystem::path(texPath).is_absolute()) {
                texPath = (gltfDir / texPath).string();
            }
            
            Texture texture(texPath.c_str(), TextureType::Emissive, 4);
            textures.push_back(std::move(texture));
        }
        
    }

    glm::mat4 modelMatrix(1.0f);

    // Find the node that references this mesh
    std::function<const aiNode*(const aiNode*)> findMeshNode = [&](const aiNode* node) -> const aiNode* {
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            if (scene->mMeshes[node->mMeshes[i]] == mesh) {
                return node;
            }
        }
        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            const aiNode* found = findMeshNode(node->mChildren[i]);
            if (found) return found;
        }
        return nullptr;
    };

    const aiNode* meshNode = findMeshNode(scene->mRootNode);
    if (meshNode) {
        aiMatrix4x4 aiMat = meshNode->mTransformation;
        const aiNode* parent = meshNode->mParent;
        while (parent) {
            aiMat = parent->mTransformation * aiMat;
            parent = parent->mParent;
        }
        // Convert aiMatrix4x4 to glm::mat4
        modelMatrix = glm::mat4(
            aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
            aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
            aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
            aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
        );
    }


    MaterialProperties matProps;
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // Get base color factor
    aiColor4D baseColorFactor;
    if (material->Get(AI_MATKEY_BASE_COLOR, baseColorFactor) == AI_SUCCESS) {
        matProps.baseColorFactor = glm::vec4(baseColorFactor.r, baseColorFactor.g, baseColorFactor.b, baseColorFactor.a);
        std::cout << "Base color factor: (" << baseColorFactor.r << ", " << baseColorFactor.g << ", " << baseColorFactor.b << ", " << baseColorFactor.a << ")" << std::endl;
    }

    // Get alpha cutoff
    float alphaCutoff = 0.5f;
    if (material->Get(AI_MATKEY_OPACITY, alphaCutoff) == AI_SUCCESS) {
        matProps.alphaCutoff = alphaCutoff;
        std::cout << "Alpha cutoff: " << alphaCutoff << std::endl;
        matProps.alphaMode_MASK = true;
    }

    // Get metallic factor
    float metallicFactor = 1.0f;
    if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) == AI_SUCCESS) {
        matProps.metallicFactor = metallicFactor;
        std::cout << "Metallic factor: " << metallicFactor << std::endl;
    }

    // Get roughness factor
    float roughnessFactor = 1.0f;
    if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor) == AI_SUCCESS) {
        matProps.roughnessFactor = roughnessFactor;
        std::cout << "Roughness factor: " << roughnessFactor << std::endl;
    }

    // Check for double sided
    int twoSided = 0;
    if (material->Get(AI_MATKEY_TWOSIDED, twoSided) == AI_SUCCESS) {
        matProps.doubleSided = (twoSided != 0);
        std::cout << "Double sided: " << matProps.doubleSided << std::endl;
    } else {
        matProps.doubleSided = true;  // Force double-sided if not specified
    }
    
    if (mesh->HasTangentsAndBitangents()) {
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            tangents.emplace_back(glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z));
            bitangents.emplace_back(glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z));
        }
    }



    // Pass modelMatrix to Model constructor
    return Model(vertices, indices, colors, textures, normals, uvs, tangents, bitangents, modelMatrix, matProps);
}

void Scene::addModel(Model&& model) { // Accept Model by move
    models.emplace_back(std::move(model)); // Use emplace_back with move
}

void Scene::setCamera(const Camera& camera) {
    this->camera = camera;
}

Camera& Scene::getCamera() {
    return this->camera;
}

void Scene::draw(Shader& shader) {

    if(skybox && skyboxShader) {
        skybox->draw(*skyboxShader, camera);
    }
    shader.activate();
    lightManager.updateShaderUniforms(shader);
    glm::vec3 camPos = camera.getPosition();
    shader.setVec3("cameraPos", glm::value_ptr(camPos));

    for (Model& model : models) {
        model.draw(shader, camera);
    }
    shader.deactivate();
}

void Scene::setSkybox(const std::string& directory) {
    std::vector<std::string> skyboxFilePaths;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            skyboxFilePaths.push_back(entry.path().string());
        }
    }
    if (skyboxFilePaths.size() != 6) {
        std::cerr << "Skybox directory must contain exactly 6 files." << std::endl;
    }
    std::sort(skyboxFilePaths.begin(), skyboxFilePaths.end()); // Ensure consistent order
    skybox = std::make_unique<Skybox>(skyboxFilePaths);
}

void Scene::setSkyboxShader(const std::string& vertexPath, const std::string& fragmentPath) {
    skyboxShader = std::make_unique<Shader>(vertexPath.c_str(), fragmentPath.c_str());
}

size_t Scene::addDirectionalLight(const glm::vec3& direction, const glm::vec3& color, float intensity) {
    return lightManager.addDirectionalLight(direction, color, intensity);
}

size_t Scene::addPointLight(const glm::vec3& position, const glm::vec3& color, float intensity) {
    return lightManager.addPointLight(position, color, intensity);
}

size_t Scene::addSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color, float intensity, float innerCutoff, float outerCutoff) {
    return lightManager.addSpotLight(position, direction, color, intensity, innerCutoff, outerCutoff);
}

void Scene::enableShadowsForLight(size_t lightIndex, unsigned int resolution) {
    if (lightIndex >= lightManager.getLightCount()) {
        std::cerr << "Invalid light index: " << lightIndex << std::endl;
        return;
    }
    
    const Light& light = lightManager.getLight(lightIndex);
    shadowManager.addShadowMap(lightIndex, light.getType(), resolution);
}

void Scene::disableShadowsForLight(size_t lightIndex) {
    shadowManager.removeShadowMap(lightIndex);
}

void Scene::setSceneBounds(const glm::vec3& center, float radius) {
    shadowManager.setSceneBounds(center, radius);
}

void Scene::drawWithShadows(Shader& shader, Shader& shadowShader) {
    // First pass: Render shadow maps
    shadowManager.renderShadowMaps(lightManager, *this, shadowShader);
    
    // Second pass: Render scene with shadows
    
    // Draw skybox first (if present)
    if(skybox && skyboxShader) {
        skybox->draw(*skyboxShader, camera);
    }
    
    // Activate main shader for scene rendering
    shader.activate();
    
    // Upload light uniforms to the shader
    lightManager.updateShaderUniforms(shader);
    
    // Set camera position for lighting calculations
    glm::vec3 camPos = camera.getPosition();
    shader.setVec3("cameraPos", glm::value_ptr(camPos));
    
    // Bind shadow maps for use in fragment shader
    shadowManager.bindShadowMapsForRendering(shader);
    
    // Draw all models in the scene
    for (Model& model : models) {
        model.draw(shader, camera);
    }
    
    shader.deactivate();
}

void Scene::calculateSceneBounds() {
    sceneMin = glm::vec3(FLT_MAX);
    sceneMax = glm::vec3(-FLT_MAX);
    
    for (const auto& model : models) {
        // Get model's vertices and transform them by model matrix
        const auto& vertices = model.getVertices(); // You'll need to add this getter
        glm::mat4 modelMatrix = model.getModelMatrix();
        
        for (const auto& vertex : vertices) {
            // Transform vertex position by model matrix
            glm::vec4 worldPos = modelMatrix * glm::vec4(vertex, 1.0f);
            glm::vec3 pos = glm::vec3(worldPos);
            
            // Update min/max bounds
            sceneMin.x = std::min(sceneMin.x, pos.x);
            sceneMin.y = std::min(sceneMin.y, pos.y);
            sceneMin.z = std::min(sceneMin.z, pos.z);
            
            sceneMax.x = std::max(sceneMax.x, pos.x);
            sceneMax.y = std::max(sceneMax.y, pos.y);
            sceneMax.z = std::max(sceneMax.z, pos.z);
        }
    }
    
    // Calculate center and radius
    calculatedSceneCenter = (sceneMin + sceneMax) * 0.5f;
    glm::vec3 extent = sceneMax - sceneMin;
    calculatedSceneRadius = glm::length(extent) * 0.5f;
    
    sceneBoundsCalculated = true;
    
    std::cout << "=== Calculated Scene Bounds ===" << std::endl;
    std::cout << "Min: (" << sceneMin.x << ", " << sceneMin.y << ", " << sceneMin.z << ")" << std::endl;
    std::cout << "Max: (" << sceneMax.x << ", " << sceneMax.y << ", " << sceneMax.z << ")" << std::endl;
    std::cout << "Center: (" << calculatedSceneCenter.x << ", " << calculatedSceneCenter.y << ", " << calculatedSceneCenter.z << ")" << std::endl;
    std::cout << "Radius: " << calculatedSceneRadius << std::endl;
}

void Scene::printSceneBounds() const {
    if (sceneBoundsCalculated) {
        std::cout << "Scene Center: (" << calculatedSceneCenter.x << ", " << calculatedSceneCenter.y << ", " << calculatedSceneCenter.z << ")" << std::endl;
        std::cout << "Scene Radius: " << calculatedSceneRadius << std::endl;
    } else {
        std::cout << "Scene bounds not calculated yet!" << std::endl;
    }
}
