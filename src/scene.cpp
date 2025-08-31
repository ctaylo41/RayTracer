#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <string>
#include "scene.h"
#include <filesystem>

Scene::Scene(const char* path) {
    loadGLTF(path);
}

bool Scene::loadGLTF(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
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
        glm::vec3 position(0.0f, 0.0f, 3.0f);
        glm::vec3 up(0.0f, 1.0f, 0.0f);
        float yaw = -90.0f;
        float pitch = 0.0f;
        float fov = 45.0f;
        float nearPlane = 0.1f;
        float farPlane = 100.0f;
        unsigned int width = 1200;
        unsigned int height = 800;

        Camera cam(position, up, yaw, pitch, fov, farPlane, nearPlane, width, height);
        setCamera(cam);
    }

    return true;
}

Model Scene::assimpMeshToModel(aiMesh* mesh, const aiScene* scene, const std::string& gltfFilePath) {
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> colors;

    // Vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        glm::vec3 vertex;
        vertex.x = mesh->mVertices[i].x;
        vertex.y = mesh->mVertices[i].y;
        vertex.z = mesh->mVertices[i].z;
        vertices.push_back(vertex);
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
        for (int type = aiTextureType_NONE; type <= aiTextureType_UNKNOWN; ++type) {
            for (unsigned int i = 0; i < material->GetTextureCount((aiTextureType)type); ++i) {
                aiString str;
                material->GetTexture((aiTextureType)type, i, &str); // Fill str with texture path
                TextureType texType = TextureType::Unknown;
                switch ((aiTextureType)type) {
                    case aiTextureType_DIFFUSE: texType = TextureType::Diffuse; break;
                    case aiTextureType_SPECULAR: texType = TextureType::Specular; break;
                    case aiTextureType_NORMALS: texType = TextureType::Normal; break;
                    case aiTextureType_EMISSIVE: texType = TextureType::Emissive; break;
                    case aiTextureType_METALNESS: texType = TextureType::Metallic; break;
                    case aiTextureType_DIFFUSE_ROUGHNESS: texType = TextureType::Roughness; break;
                    case aiTextureType_LIGHTMAP: texType = TextureType::Occlusion; break;
                    default: texType = TextureType::Unknown; break;
                }

                std::filesystem::path gltfDir = std::filesystem::path(gltfFilePath).parent_path();
                std::string texPath = str.C_Str();

                if (!std::filesystem::path(texPath).is_absolute()) {
                    texPath = (gltfDir / texPath).string();
                }
                
                Texture texture(texPath.c_str(), texType, 0);
                textures.push_back(texture);
            }
        }
    }
    return Model(vertices, indices, colors, textures, normals, uvs);
}

void Scene::addModel(Model&& model) { // Accept Model by move
    models.emplace_back(std::move(model)); // Use emplace_back with move
}

void Scene::setCamera(const Camera& camera) {
    this->camera = camera;
}

const Camera& Scene::getCamera() const {
    return this->camera;
}

void Scene::draw(Shader& shader) {
    for (Model& model : models) {
        model.draw(shader, camera);
    }
}