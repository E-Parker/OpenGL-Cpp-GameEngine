#pragma once

#include <glad/glad.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Uniform {
    // Structure to store an OpenGL Uniform.
    char* Alias;        // Name of the uniform.
    char* AliasEnd;
    uint16_t Type;
    uint16_t Size;      // byte size of each element.
    GLint Elements;     // value of 1 means it is not an array.
} Uniform;

// "Constructor" for the Uniform.
Uniform* init_uniform(const GLenum Type, const GLuint elements, const char* name, const int length);

#define uniform_data(u) ((void*)((char*)u + sizeof(Uniform)))                               // Data stored by the uniform.
#define uniform_data_size(u) (uint64_t)(((Uniform*)u)->Size * ((Uniform*)u)->Elements)      // Size of a uniform's data in bytes. This will vary depending on the type stored there.

typedef struct Shader {
    char* Alias;        // Name of the shader.
    char* AliasEnd; 
    GLuint Program;     // Location of the shader program on the GPU.
    GLint UniformCount; // Number of uniforms used by the shader.
    Uniform** Uniforms; // Array of uniforms expected by the program.    
    uint64_t* Lookup;   // Lookup table for Uniforms.
} Shader;

Shader* CreateShader(const GLuint program, const char* alias);
void FreeShader(Shader** shader);
void GetUniform(const Shader* shader, const char* alias, Uniform** outVal);

#ifdef __cplusplus
}
#endif