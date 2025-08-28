#ifndef EBO_CLASS
#define EBO_CLASS
#include <vector>
#include <glad/glad.h>

class ElementBufferObject {
public:
    //Constructor and Deconstructor for EBO
    ElementBufferObject(const std::vector<unsigned int>& indices);
    ~ElementBufferObject();

    //Bind and Unbind
    void bind() const;
    void unbind() const;
private:
    //render id
    GLuint renderID;
    std::vector<unsigned int> indices;
};

#endif