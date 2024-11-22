#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void CreateShader(GLuint* shader, GLint type, const char* path);
GLuint CreateProgram(GLuint vs, GLuint fs);
