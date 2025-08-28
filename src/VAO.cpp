#include "VAO.h"

VertexArrayObject::VertexArrayObject() {
    glGenVertexArrays(1, &renderID);
}

VertexArrayObject::~VertexArrayObject() {
    glDeleteVertexArrays(1, &renderID);
}

void VertexArrayObject::bind() const {
    glBindVertexArray(renderID);
}

void VertexArrayObject::unbind() const {
    glBindVertexArray(0);
}

void VertexArrayObject::linkAttrib(VertexBufferObject& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    VBO.bind();
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    VBO.unbind();
}
