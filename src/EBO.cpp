#include "EBO.h"

ElementBufferObject::ElementBufferObject(const std::vector<unsigned int>& indices) {
    //Generate EBO buffer for renderID
    glGenBuffers(1, &renderID);
    //Bind the EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderID);
    //Upload the index data to the GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

ElementBufferObject::~ElementBufferObject() {
    //Delete the EBO
    glDeleteBuffers(1, &renderID);
}

void ElementBufferObject::bind() const {
    //Bind the EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderID);
}

void ElementBufferObject::unbind() const {
    //Unbind the EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}