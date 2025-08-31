#include "VBO.h"

VertexBufferObject::VertexBufferObject(const std::vector<Vertex>& vertices) : vertices(vertices) {
    glGenBuffers(1, &renderID);
    glBindBuffer(GL_ARRAY_BUFFER, renderID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

VertexBufferObject::~VertexBufferObject() {
    glDeleteBuffers(1, &renderID);
}

void VertexBufferObject::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, renderID);
}

void VertexBufferObject::unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}