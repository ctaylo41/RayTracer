#ifndef VBO_CLASS
#define VBO_CLASS

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 uv;
};

class VertexBufferObject {
public:
    // Constructor and Destructor for VBO
    VertexBufferObject(const std::vector<Vertex>& vertices);
    ~VertexBufferObject();

    // Bind and Unbind
    void bind() const;
    void unbind() const;

private:
    GLuint renderID;
    std::vector<Vertex> vertices;
};

#endif