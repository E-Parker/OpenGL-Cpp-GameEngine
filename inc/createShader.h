#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

char* CreateShader(GLuint* shader, GLint type, const char* path);
GLuint CreateProgram(GLuint vs, GLuint fs);