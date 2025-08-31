#ifndef TEXTURE_CLASS
#define TEXTURE_CLASS

#include <glad/glad.h>
#include <stb_image.h>
#include <string>

enum class TextureType {
	Diffuse,
	Specular,
	Normal,
	Emissive,
	Metallic,
	Roughness,
	Occlusion,
	Unknown
};

#include "shader.h"

class Texture {
public:
	GLuint ID;
	TextureType type;
	GLuint unit;

	Texture(const char* image, TextureType texType, GLuint slot);

	void texUnit(Shader& shader, const char* uniform, GLuint unit);

	void bind();
	void unbind();

	~Texture();
};
#endif