#ifndef VAO_CLASS
#define VAO_CLASS

#include "VBO.h"

#include <glad/glad.h>
#include <vector>
#include <string>
#include <iostream>

class VertexArrayObject {
public:
    // Constructor and Destructor for VAO
    VertexArrayObject();
    ~VertexArrayObject();

    std::string getID() const;


    void linkAttrib(VertexBufferObject& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);

    // Bind and Unbind
    void bind() const;
    void unbind() const;

private:
    GLuint renderID;
};

#endif