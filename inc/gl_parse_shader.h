#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

char* internal_ReadShaderSource(const char* path);
char* internal_LoadShaderIncludes(const char* path);

void CompileShader(GLuint* shader, GLint type, const char* path);
GLuint CompileShaderProgram(GLuint vs, GLuint fs);
