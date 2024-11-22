#pragma once

#include <glad/glad.h>

typedef struct Uniform {
    // Structure to store an OpenGL Uniform.
    char* Alias;
    char* AliasEnd;
    GLint Size;
    GLenum Type;
    void* Data;
} Uniform;


typedef struct Shader {
    GLuint Program;     // Location of the shader program on the GPU.
    GLint UniformCount; // Number of uniforms used by the shader.
    char* Alias;        // Name of the shader.
    Uniform* Uniforms;  // Array of uniforms expected by the program.    
    void* Data;         // Raw data of each uniform in a large block.
    uint64_t* Lookup;   // Lookup table for Uniforms. 
    // This allows uniforms to be stored in order while allowing for a hashtable for fast retrieval.

} Shader;

Shader* CreateShader(GLuint program, const char* alias);
void FreeShader(Shader** shader);

void GetUniform(const Shader* shader, const char* alias, Uniform** outVal);
