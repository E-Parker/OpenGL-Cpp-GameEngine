#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <glad/glad.h>

// simple macro to get a pointer to the end of the alias.
#define Uniform_alias_end(uniform) (uniform->Alias + uniform->AliasLength + 1)

// using #define here to keep it the same as OpenGL.
#define UNIFORM_TYPE_GENERIC 0x0001
#define UNIFORM_TYPE_SINGLE 0x0002
#define UNIFORM_TYPE_BUFFER_ITEM 0x0003
#define UNIFORM_TYPE_BUFFER 0x0004
#define UNIFORM_TYPE_SAMPLER 0x0005

typedef struct UniformGeneric {
    // Template body for any uniform type
    #define UNIFORM_BODY()\
    char* Alias;\
    uint32_t AliasLength;\
    uint32_t UniformType;\
    
    UNIFORM_BODY()
    uint64_t padding0;
    uint64_t padding1;

} UniformGeneric;

typedef struct UniformInformation {
    // GENERIC DEFINITIONS:
    UNIFORM_BODY()
    GLint Location;
    GLenum Type;
    GLint Elements;

    // UNIFORM IN BLOCK:
    GLint BlockOffset;  // byte offset of the item in the block. (-1 if not in a block.)
 
} UniformInformation;

// SHADER UNIFORM SAMPLER
//
//

typedef struct UniformSampler {
    UNIFORM_BODY()
    uint16_t Location;
    uint16_t Type;
    uint16_t width;
    uint16_t height;
    uint8_t filterType;
    uint8_t channels;
} UniformSampler;


//  SHADER UNIFORM
//
//

typedef struct Uniform {
    // Structure to store an OpenGL Uniform. Same size as UniformBuffer struct
    UNIFORM_BODY()
    uint16_t Type;      // GLenum type of the uniform. Needed to do upload.
    uint16_t Size;      // byte size of each element.
    uint16_t Elements;  // value of 1 means it is not an array.
    uint16_t Offset;    // Only meaningful for uniforms that are part of structures.
    void* Data;
} Uniform;

Uniform* internal_UniformBuffer_item_create(const UniformInformation* info, void* sharedBuffer);
Uniform* internal_Uniform_create(const UniformInformation* info);

#define Uniform_exec_if_type(UniformType, ...) ( if((Uniform*)uniform->UniformType == UniformType){ __VA_ARGS__ })
#define Uniform_get_data(T, uniform) ((T*)((Uniform*)uniform->Data))
#define Uniform_get_data_size(uniform) (uint64_t)(uniform->Size * uniform->Elements)
#define Uniform_set_data(uniform, data) (memcpy(Uniform_get_data(uint8_t, uniform), data, Uniform_get_data_size(uniform)))

//  SHADER UNIFORM BUFFER / SHADER STORAGE BUFFER
//
//

typedef struct UniformBuffer {
    // Structure to store an OpenGL buffer. Same size as Uniform struct
    UNIFORM_BODY()
    uint16_t Size;          // Total byte size of the buffer.
    uint16_t BindingIndex;  // binding index of the buffer on the GPU.
    GLint BufferObject;
    uint64_t References;
    HashTable* Uniforms;
} UniformBuffer;

void UniformBuffer_destroy(UniformBuffer** buffer);
void internal_UniformBuffer_set_region(const UniformBuffer* buffer, const uint64_t byteIndex, const uint64_t regionSizeInBytes, const void* data);
void internal_UniformBuffer_set_all(const UniformBuffer* buffer, const void* data);
void internal_UniformBuffer_set(const UniformBuffer* buffer, const char* alias, void* data);
void UniformBuffer_get_Uniform(const UniformBuffer* buffer, const char* alias, Uniform** outVal);

UniformBuffer* UniformBuffer_get_self(const char* alias);

void UniformBuffer_update_all();

#define UniformBuffer_get_shared(buffer) ((void*)((uint8_t*)buffer + sizeof(UniformBuffer)))
#define UniformBuffer_set(buffer, alias, Value) (internal_UniformBuffer_set(buffer, alias, (void*)Value))
#define UniformBuffer_set_Global(bufferAlias, alias, Value) (UniformBuffer_set(UniformBuffer_get_self(bufferAlias), alias, Value))

//  SHADER
//
//

typedef struct Shader {
    char* Alias;                // Name of the shader.
    char* AliasEnd;
    GLuint Program;             // Location of the shader program on the GPU.
    HashTable* Uniforms;        // Table of uniforms.
    HashTable* UniformBuffers;  // Table of uniform buffers.
    uint64_t References;
} Shader;

void InitShaders();
void Shader_use(const Shader* shader);
void DereferenceShaders();

GLint internal_Program_uniform_count(const GLuint program);
void internal_Program_uniform_parse(const GLuint program, HashTable* table);
void internal_Program_buffer_parse(const GLuint program, HashTable* table);
static void internal_Program_buffer_uniform_parse(const GLuint program, const uint16_t uniformCount, const GLint* indicies, UniformBuffer* uniformBuffer);


Shader* Shader_create(const GLuint program, const char* alias);
void Shader_destroy(Shader** shader);

void Shader_set_uniform(const Shader* shader, const char* alias, void* data);
void Shader_set_uniformBuffer(const Shader* shader, const char* alias, void* data);

void Shader_get_uniform(const Shader* shader, const char* alias, Uniform** outVal);
void Shader_get_uniformBuffer(const Shader* shader, const char* alias, UniformBuffer** outVal);

#define Shader_get_uniform_count(shader) (shader->Uniforms->SlotsUsed)
#define Shader_get_buffer_count(shader) (shader->UniformBuffers->SlotsUsed)

#ifdef __cplusplus
}
#endif
