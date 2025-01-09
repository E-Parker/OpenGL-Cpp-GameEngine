#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>

#include "gl_parse_shader.h"
#include "cStringUtilities.h"

#define GL_ERROR_LOG_SIZE 512


char* internal_ReadShaderSource(const char* path) {

    std::stringstream stream;

    try {
        // Load text file
        std::ifstream file;
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);    // Set exceptions for try catch.
        file.open(path);                                                    // Attempt to open the file.

        // Interpret the file as a giant string
        stream << file.rdbuf();
        file.close();

    }
    catch (std::ifstream::failure& e) {
        std::cout << "Shader (" << path << ") not found: " << e.what() << std::endl;
        return nullptr;
    }

    // Get the raw file as a string to compile
    std::string str = stream.str();
    char* src = new char[str.length() + 1];
    memcpy(src, str.c_str(), str.length() + 1);

    return src;
}

char* internal_LoadShaderIncludes(const char* path) {
    //TODO: implement a way to copy the source of included shaders. it wont be efficient but whatever.
    return nullptr;
}


void CompileShader(GLuint* shader, GLint type, const char* path) {
    
    char* src = internal_ReadShaderSource(path);

    // Compile text as a shader
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &src, NULL);
    glCompileShader(*shader);

    // Check for compilation errors
    GLint success;
    GLchar infoLog[GL_ERROR_LOG_SIZE];
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        glGetShaderInfoLog(*shader, GL_ERROR_LOG_SIZE, NULL, infoLog);
        std::cout << "Shader failed to compile: \n" << infoLog << std::endl;
    }

    delete[] src;
}


GLuint CompileShaderProgram(GLuint vs, GLuint fs) {
    /* This function compiles a vertex and fragment shader into a program to run on the GPU. */
    
    // Set up a new shader program and compile it.
    GLuint program = glCreateProgram(); // Create a new empty program.
    glAttachShader(program, vs);        // Attach the vertex shader.
    glAttachShader(program, fs);        // Attach the fragment shader.
    glLinkProgram(program);             // Run the linking step.

    // Find out if there were errors linking the shader program.
    int success;
    int vs_success;
    int fs_success;

    int dummyLength;
    char progLog[GL_ERROR_LOG_SIZE]{'\0'};
    char vsLog[GL_ERROR_LOG_SIZE]{'\0'};
    char fsLog[GL_ERROR_LOG_SIZE]{'\0'};
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    

    // If the compile failed for some reason, clear the program and log the error.
    if (!success) {
        glGetProgramInfoLog(program, GL_ERROR_LOG_SIZE, &dummyLength, progLog);
        std::cout << "ERROR: Could not link a shader program! Program error Log:\n" << progLog << std::endl;
        program = GL_NONE;
        
        // Debug the vertex shader and fragment shader.
        glGetShaderiv(vs, GL_COMPILE_STATUS, &vs_success);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &fs_success);

        if (!vs_success) {
            glGetShaderInfoLog(vs, GL_ERROR_LOG_SIZE, &dummyLength, vsLog);
            std::cout << "vertex shader Log:\n" << vsLog << std::endl;
        }

        if (!fs_success) {
            glGetShaderInfoLog(fs, GL_ERROR_LOG_SIZE, &dummyLength, fsLog);
            std::cout << "fragment shader Log:\n" << fsLog << std::endl;
        }
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}
