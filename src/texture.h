#ifndef TEXTURE_CLASS
#define TEXTURE_CLASS

#include<glad/glad.h>
#include<stb_image.h>

#include"shader.h"

class Texture
{
public:
	GLuint ID;
	const char* type;
	GLuint unit;

	Texture(const char* image, const char* texType, GLuint slot);

	void texUnit(Shader& shader, const char* uniform, GLuint unit);

	void bind();
    
	void unbind();

	~Texture();
};
#endif