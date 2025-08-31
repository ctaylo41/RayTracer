#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <iostream>
#include <glad/glad.h>

void checkGLError(const std::string& operation);

#endif // ERROR_H