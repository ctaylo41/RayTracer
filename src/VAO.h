#ifndef VAO_CLASS
#define VAO_CLASS

#include "VBO.h"

#include <glad/glad.h>
#include <vector>

class VertexArrayObject {
public:
    // Constructor and Destructor for VAO
    VertexArrayObject();
    ~VertexArrayObject();

    void linkAttrib(VertexBufferObject& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);

    // Bind and Unbind
    void bind() const;
    void unbind() const;

private:
    GLuint renderID;
    std::vector<float> vertices;
};

#endif