#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <glad/glad.h>
#include <cStringUtilities.h>

//  SHADER UNIFORM
//
//

typedef struct UniformInformation {
    bool isBuffer;
    char* Alias;
    char* AliasEnd;
    GLint Location;
    GLenum Type;
    GLint Elements;
    GLint BlockIndex;   // Index of the uniform block. (-1 if not a block.)
    GLint BlockOffset;  // Start byte of the block. (-1 if not a block.)
} UniformInformation;


typedef struct UniformBuffer {
    // Structure to store an OpenGL buffer. Same size as Uniform struct
    bool isBuffer;
    char* Alias;        // Name of the uniform buffer.
    char* AliasEnd;     
    uint16_t Size;      // Total byte size of the buffer
    uint16_t BindingIndex;
    GLint BufferObject; // OpenGL buffer.
} UniformBuffer;


typedef struct Uniform {
    // Structure to store an OpenGL Uniform. Same size as UniformBuffer struct
    bool isBuffer;
    char* Alias;        // Name of the uniform.
    char* AliasEnd;
    uint16_t Type;      // GLenum type of the uniform. Needed to do upload.
    uint16_t Size;      // byte size of each element.
    GLint Elements;     // value of 1 means it is not an array.
} Uniform;


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


UniformBuffer* UniformBuffer_create (const UniformInformation* info, const uint64_t size);
void UniformBuffer_destroy(UniformBuffer** buffer);
void internal_UniformBuffer_set_region(const UniformBuffer* buffer, const uint64_t byteIndex, const uint64_t regionSizeInBytes, const void* data);
void internal_UniformBuffer_set_all(const UniformBuffer* buffer, const void* data);
void UniformBuffer_get(const char* alias, UniformBuffer** outVal);

#define UniformBuffer_set_at(type, buffer, index, data) (internal_UniformBuffer_set_region(buffer, index, ((UniformBuffer*)buffer->Size) * buffer->Stride), data))
#define UniformBuffer_set_region(type, buffer, fromIndex, toIndex, data) (internal_UniformBuffer_set_region(buffer, fromIndex, ((UniformBuffer*)buffer->Size) * (toIndex - fromIndex) * buffer->Stride), data))
#define UniformBuffer_set_all(type, buffer, data)


Uniform* internal_Uniform_create(const UniformInformation* info);
#define Uniform_get_data(T, u) ((T*)((char*)u + sizeof(Uniform)))                                   // Data stored by the uniform.
#define Uniform_get_data_size(u) (uint64_t)(((Uniform*)u)->Size * ((Uniform*)u)->Elements)          // Size of a uniform's data in bytes. This will vary depending on the type stored there.
#define Uniform_set_data(u, d) (memcpy(((char*)u + sizeof(Uniform)), d, ((Uniform*)u)->Size))       // Set the data stored by uniform.

void InitShaders();
void DereferenceShaders();

GLint internal_Program_uniform_count(const GLuint program);
UniformInformation* internal_Program_uniform_parse(const GLuint program);

Shader* Shader_create(const GLuint program, const char* alias);
void Shader_destroy(Shader** shader);
void Shader_set_uniform(const Shader* shader, const char* alias, void* data);
void Shader_set_uniformBuffer(const Shader* shader, const char* alias, void* data);

void internal_Shader_get_uniform(const Shader* shader, const char* alias, void** outVal);
void Shader_get_uniform(const Shader* shader, const char* alias, Uniform** outVal);
void Shader_get_uniformBuffer(const Shader* shader, const char* alias, UniformBuffer** outVal);


#ifdef __cplusplus
}
#endif
