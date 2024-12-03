#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <glad/glad.h>
#include <cStringUtilities.h>

//  SHADER UNIFORM BUFFER
//
//

typedef struct UniformBuffer {
    // Structure to store an OpenGL buffer.
    char* Alias;        // Name of the uniform buffer.
    char* AliasEnd;     
    GLint BufferObject; // OpenGL buffer.
    uint16_t Size;      // Total byte size of the buffer
    uint16_t BindingIndex;
} UniformBuffer;

UniformBuffer* UniformBuffer_create (const char* alias, const uint64_t size);
void UniformBuffer_destroy(UniformBuffer** buffer);
void internal_UniformBuffer_set_region(const UniformBuffer* buffer, const uint64_t byteIndex, const uint64_t regionSizeInBytes, const void* data);
void internal_UniformBuffer_set_all(const UniformBuffer* buffer, const void* data);

#define UniformBuffer_set_at(type, buffer, index, data) (internal_UniformBuffer_set_region(buffer, index, ((UniformBuffer*)buffer->Size) * buffer->Stride), data))
#define UniformBuffer_set_region(type, buffer, fromIndex, toIndex, data) (internal_UniformBuffer_set_region(buffer, fromIndex, ((UniformBuffer*)buffer->Size) * (toIndex - fromIndex) * buffer->Stride), data))
#define UniformBuffer_set_all(type, buffer, data)

//  SHADER UNIFORM
//
//

typedef struct Uniform {
    // Structure to store an OpenGL Uniform. Intended for internal use.
    char* Alias;        // Name of the uniform.
    char* AliasEnd;
    uint16_t Type;      // GLenum type of the uniform. Needed to do upload.
    uint16_t Size;      // byte size of each element.
    GLint Elements;     // value of 1 means it is not an array.
} Uniform;

// "Constructor" for the Uniform.
Uniform* internal_Uniform_create(const GLenum Type, const GLuint elements, const char* name, const int length);
#define Uniform_get_data(T, u) ((T*)((char*)u + sizeof(Uniform)))                                   // Data stored by the uniform.
#define Uniform_get_data_size(u) (uint64_t)(((Uniform*)u)->Size * ((Uniform*)u)->Elements)          // Size of a uniform's data in bytes. This will vary depending on the type stored there.
#define Uniform_set_data(u, d) (memcpy(((char*)u + sizeof(Uniform)), d, ((Uniform*)u)->Size))       // Set the data stored by uniform.

//  SHADER
//
//

typedef struct Shader {
    char* Alias;        // Name of the shader.
    char* AliasEnd; 
    GLuint Program;     // Location of the shader program on the GPU.
    GLint UniformCount; // Number of uniforms used by the shader.
    Uniform** Uniforms; // Array of uniforms expected by the program.    
    uint64_t* Lookup;   // Lookup table for Uniforms.
} Shader;


GLint internal_Program_uniform_count(const GLuint program);

Shader* Shader_create(const GLuint program, const char* alias);
void Shader_destroy(Shader** shader);
void Shader_set_uniform(const Shader* shader, const char* alias, void* data);
void Shader_get_uniform(const Shader* shader, const char* alias, Uniform** outVal);

#ifdef __cplusplus
}
#endif
