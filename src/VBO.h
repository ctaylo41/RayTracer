#ifndef VBO_CLASS
#define VBO_CLASS

#include <vector>
#include <glad/glad.h>

class VertexBufferObject {
public:
    // Constructor and Destructor for VBO
    VertexBufferObject(const std::vector<float>& vertices);
    ~VertexBufferObject();

    // Bind and Unbind
    void bind() const;
    void unbind() const;

private:
    GLuint renderID;
    std::vector<float> vertices;
};

#endif