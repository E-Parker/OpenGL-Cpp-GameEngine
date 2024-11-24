#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void CompileShader(GLuint* shader, GLint type, const char* path);
GLuint CompileShaderProgram(GLuint vs, GLuint fs);
