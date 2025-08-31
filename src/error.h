#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <iostream>
#include <glad/glad.h>
void checkError(const std::string& callLocation) {
    GLenum error;
    bool foundError = false;
    while ((error = glGetError()) != GL_NO_ERROR) {
        foundError = true;
        std::cerr << "OpenGL Error during " << callLocation << ": " << error << std::endl;
        switch(error) {
            case GL_INVALID_ENUM: std::cerr << "GL_INVALID_ENUM" << std::endl; break;
            case GL_INVALID_VALUE: std::cerr << "GL_INVALID_VALUE" << std::endl; break;
            case GL_INVALID_OPERATION: std::cerr << "GL_INVALID_OPERATION" << std::endl; break;
            case GL_OUT_OF_MEMORY: std::cerr << "GL_OUT_OF_MEMORY" << std::endl; break;
            default: std::cerr << "Unknown error" << std::endl; break;
        }
    }
}

#endif // ERROR_H