#include "error.h"
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