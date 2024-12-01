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
    uint16_t Size;      // Byte size of items in the buffer.
    uint16_t Stride;    // number of bytes per set of items. (i.e. Vector3 -> float x, float y, float z)
    uint16_t BufferSize;// total number of bytes for the buffer.
    uint16_t Type;      // GLenum type to be stored.
} UniformBuffer;

UniformBuffer* internal_UniformBuffer_create(const GLenum usage, const char* alias, const uint64_t itemSize, const uint16_t stride, const uint16_t elements);
void UniformBuffer_destroy(UniformBuffer** buffer);
void internal_UniformBuffer_set_at(UniformBuffer* buffer, const uint16_t index, const uint64_t itemSize, const void* data); 
void internal_UniformBuffer_set_region(UniformBuffer* buffer, const uint16_t fromIndex, const uint16_t toIndex, const uint64_t regionSize, const void* data);
void internal_UniformBuffer_set_all(UniformBuffer* buffer, const uint64_t itemSize, const void* data);

#define UniformBuffer_set_at(type, buffer, index, data) (internal_UniformBuffer_set_at(buffer, index, sizeof(type), data))
#define UniformBuffer_set_region(type, buffer, fromIndex, toIndex, data) (internal_UniformBuffer_set_region(buffer, fromIndex, toIndex, (sizeof(type) * (toIndex - fromIndex) * buffer->Stride), data))
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
#define Uniform_get_data(u) ((void*)((char*)u + sizeof(Uniform)))                               // Data stored by the uniform.
#define Uniform_get_data_size(u) (uint64_t)(((Uniform*)u)->Size * ((Uniform*)u)->Elements)      // Size of a uniform's data in bytes. This will vary depending on the type stored there.

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
void Shader_get_uniform(const Shader* shader, const char* alias, Uniform** outVal);

#ifdef __cplusplus
}
#endif
