#include "VAO.h"
#include <GLFW/glfw3.h>

// Helper function to check OpenGL errors
void checkGLError(const std::string& operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error during " << operation << ": " << error << std::endl;
        switch(error) {
            case GL_INVALID_ENUM: std::cerr << "GL_INVALID_ENUM" << std::endl; break;
            case GL_INVALID_VALUE: std::cerr << "GL_INVALID_VALUE" << std::endl; break;
            case GL_INVALID_OPERATION: std::cerr << "GL_INVALID_OPERATION_" << std::endl; break;
            case GL_OUT_OF_MEMORY: std::cerr << "GL_OUT_OF_MEMORY" << std::endl; break;
            default: std::cerr << "Unknown error" << std::endl; break;
        }
    }
}

VertexArrayObject::VertexArrayObject() : renderID(0) {
    // Check if OpenGL context is current
    if (!glfwGetCurrentContext()) {
        std::cerr << "Error: No OpenGL context current when creating VAO!" << std::endl;
        return;
    }
    
    glGenVertexArrays(1, &renderID);
    checkGLError("glGenVertexArrays");
    
    if (renderID == 0) {
        std::cerr << "Failed to generate VAO - got ID 0!" << std::endl;
    } else {
        std::cout << "VAO created with ID: " << renderID << std::endl;
    }
}

VertexArrayObject::~VertexArrayObject() {
    if (renderID != 0) {
        glDeleteVertexArrays(1, &renderID);
        checkGLError("glDeleteVertexArrays");
    }
}

void VertexArrayObject::bind() const {
    if (renderID == 0) {
        std::cerr << "Warning: Attempting to bind VAO with ID 0!" << std::endl;
        return;
    }
    glBindVertexArray(renderID);
    checkGLError("glBindVertexArray");
}

void VertexArrayObject::unbind() const {
    glBindVertexArray(0);
    checkGLError("glBindVertexArray(0)");
}

void VertexArrayObject::linkAttrib(VertexBufferObject& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    // Make sure VAO is bound first
    bind();
    VBO.bind();
    
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    checkGLError("glVertexAttribPointer");
    
    glEnableVertexAttribArray(layout);
    checkGLError("glEnableVertexAttribArray");
    
    VBO.unbind();
}

std::string VertexArrayObject::getID() const {
    return std::to_string(renderID);
}