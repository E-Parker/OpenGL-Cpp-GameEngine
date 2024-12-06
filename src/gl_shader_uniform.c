﻿#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "cStringUtilities.h"
#include "gl_types.h"
#include "hash_table.h"
#include "gl_shader_uniform.h"

#define MAX_ALIAS_SIZE 512

static uint16_t BufferCount = 0;
static HashTable* StorageBufferTable = NULL;
static HashTable* ShaderProgramTable = NULL;
static HashTable* TextureTable = NULL;
static HashTable* ShaderCompilationTable = NULL;

void InitShaders() {
    StorageBufferTable = HashTable_create(UniformBuffer, 128);
    ShaderProgramTable = HashTable_create(Shader, 512);
    // TextureTable = HashTable_create(Texture, 512);
}

void DereferenceShaders() {
    // Function to dereference all shaders and shader objects. 
    // After this is called, all functions will fail until InitShaders() is called again.
    //

    // Destroy the uniform buffers.
    for (uint64_t i = 0; i < StorageBufferTable->Size; i++) {
        if (StorageBufferTable->Array[i].Value) {
            UniformBuffer_destroy((UniformBuffer**)&(StorageBufferTable->Array[i].Value));
        }
    }

    // Destroy the shader programs.
    for (uint64_t i = 0; i < ShaderProgramTable->Size; i++) {
        if (ShaderProgramTable->Array[i].Value) {
            Shader_destroy((Shader**)&(ShaderProgramTable->Array[i].Value));
        }
    }

    // Destroy the shader programs.
    //for (uint64_t i = 0; i < TextureTable->Size; i++) {
    //    if (TextureTable->Array[i].Value) {
    //        Texture_destroy((Texture**)&(TextureTable->Array[i].Value));
    //    }
    //}

    // Now destroy the tables which store them. 
    HashTable_destroy(&ShaderProgramTable);
    HashTable_destroy(&StorageBufferTable);
}

void UniformBuffer_destroy(UniformBuffer** buffer) {

    if (!(*buffer)) {
        return;
    }

    if (--(*buffer)->References != 0) {
        return;
    }

    if (!(*buffer)->Uniforms) {
        return;
    }

    glDeleteBuffers(1, &((*buffer)->BufferObject));

    for (HashTable_array_itterator((*buffer)->Uniforms)) {
        Uniform* uniform = HashTable_array_at(Uniform, (*buffer)->Uniforms, i);
        if (uniform) {
            free(uniform->Alias);
        }
    }

    // Call destroy first since removing from the global table will incorrectly destroy the object.
    HashTable_destroy(&(*buffer)->Uniforms);
    free((*buffer)->Alias);
    
    // DO NOT CALL FREE. Removing it from the global table will handle that. 
    //free(*buffer);
    
    // Now that it's been cleaned up, try to remove it from the global table to avoid deleting twice.
    HashTable_remove(StorageBufferTable, (*buffer)->Alias);

    *buffer = NULL;
}

void internal_UniformBuffer_set_region(const UniformBuffer* buffer, const uint64_t byteIndex, const uint64_t regionSizeInBytes, const void* data) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->BufferObject);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, byteIndex, regionSizeInBytes, data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, GL_NONE);
}

void internal_UniformBuffer_set_all(const UniformBuffer* buffer, const void* data) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->BufferObject);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, buffer->Size, data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, GL_NONE);
}


void internal_UniformBuffer_set(const UniformBuffer* buffer, const char* alias, void* data) {
    // set the value of an item in a buffer by its variable name.

    if (!buffer) {
        return;
    }

    Uniform* uniform;
    UniformBuffer_get_Uniform(buffer, alias, &uniform);

    if (uniform) {
        Uniform_set_data(uniform, data);
    }
}

void UniformBuffer_get_Uniform(const UniformBuffer* buffer, const char* alias, Uniform** outVal) {
    // Get an item in a buffer by its variable name.
    HashTable_find(buffer->Uniforms, alias, outVal);
}

UniformBuffer* UniformBuffer_get_self(const char* alias) {
    // Simple wrapper function to access the StorageBufferTable.
    assert(StorageBufferTable);
    UniformBuffer* outVal;
    HashTable_find(StorageBufferTable, alias, &outVal);
    return outVal;
}

void UniformBuffer_update_all() {
    // Upload all uniform buffers.

    for (HashTable_array_itterator(StorageBufferTable)) {
        UniformBuffer* buffer = HashTable_array_at(UniformBuffer, StorageBufferTable, i);
        if (buffer) {
            internal_UniformBuffer_set_all(buffer, UniformBuffer_get_shared(buffer));
        }
    }
}

Uniform* internal_UniformBuffer_item_create(const UniformInformation* info, void* sharedBuffer) {
    // Initialize a Uniform that is a part of a buffer. These uniforms use a shared buffer.

    // Get the byte size of the type.
    uint64_t size = size_from_gl_type(info->Type);

    // allocate enough space for the Uniform header + the space required to store it's data. 
    Uniform* newUniform = (Uniform*)malloc(sizeof(Uniform));
    assert(newUniform != NULL);
    assert(info->BlockOffset != -1);
    newUniform->Data = (void*)(((uint8_t*)sharedBuffer) + info->BlockOffset);
    newUniform->UniformType = UNIFORM_TYPE_BUFFER_ITEM;
    newUniform->Alias = info->Alias;
    newUniform->AliasLength = info->AliasLength;
    newUniform->Size = size;
    newUniform->Elements = info->Elements;
    newUniform->Type = info->Type;
    newUniform->Offset = info->BlockOffset;

    return newUniform;
}

Uniform* internal_Uniform_create(const UniformInformation* info) {
    // Internal function to initialize a Uniform* 
    // 
    //
    
    // Get the byte size of the type.
    uint64_t size = size_from_gl_type(info->Type);

    // allocate enough space for the Uniform header + the space required to store it's data. 
    Uniform* newUniform = (Uniform*)malloc(sizeof(Uniform));
    assert(newUniform != NULL);

    newUniform->Data = calloc(size,info->Elements);
    newUniform->UniformType = UNIFORM_TYPE_SINGLE;
    newUniform->Alias = info->Alias;
    newUniform->AliasLength = info->AliasLength;
    newUniform->Size = size;
    newUniform->Elements = info->Elements;
    newUniform->Type = info->Type;
    newUniform->Offset = info->BlockOffset;

    return newUniform;
}

Shader* Shader_create(const GLuint program, const char* alias) {
    /* create a new shader, populate the fields and return a pointer to it. */
    GLint uniformCount = internal_Program_uniform_count(program);
    GLint bufferCount = internal_Program_buffer_count(program);
    GLint uniformCountTotal = uniformCount + bufferCount;

    // Allocate the shader.
    Shader* shader = (Shader*)calloc(1, sizeof(Shader));
    assert(shader != NULL);

    shader->Uniforms = HashTable_create(Uniform, uniformCount);
    shader->UniformBuffers = HashTable_create(UniformBuffer, bufferCount);
   
    internal_Program_uniform_parse(program, shader->Uniforms);
    internal_Program_buffer_parse(program, shader->UniformBuffers);

    char* aliasEnd = FindBufferEnd(alias);
    uint64_t aliasLength = aliasEnd - alias + 1;

    char* shaderName = (char*)malloc(aliasLength);
    assert(shaderName != NULL);

    memcpy(shaderName, alias, aliasLength);

    shader->Program = program;
    shader->Alias = shaderName;
    shader->AliasEnd = shaderName + aliasLength - 1;
    shader->Program = program;

    return shader;

}

void Shader_destroy(Shader** shader){
    /* Free a shader allocated with CreateShader. */
    
    glDeleteProgram((*shader)->Program);
    (*shader)->Program = GL_NONE;

    for (HashTable_array_itterator((*shader)->Uniforms)) {
        Uniform* uniform = HashTable_array_at(Uniform, (*shader)->Uniforms, i);
        if (uniform) {
            free(uniform->Alias);
            free(uniform->Data);
            (*shader)->Uniforms->Array[i].Value = NULL;
        }
    }

    for (HashTable_array_itterator((*shader)->UniformBuffers)) {
        UniformBuffer_destroy(&(UniformBuffer*)((*shader)->UniformBuffers->Array[i].Value));
    }

    free((*shader)->Alias);
    free((*shader));
    *shader = NULL;
}

void Shader_get_uniform(const Shader* shader, const char* alias, Uniform** outVal) {
    HashTable_find(shader->Uniforms, alias, outVal);
}

void Shader_get_uniformBuffer(const Shader* shader, const char* alias, UniformBuffer** outVal) {
    HashTable_find(shader->UniformBuffers, alias, outVal);
}

void Shader_set_uniform(const Shader* shader, const char* alias, void* data) {
    /* Set the value of a Shader's Uniform by the variable name. */

    Uniform* uniform;
    Shader_get_uniform(shader, alias, &uniform);

    if (uniform != NULL) {
        Uniform_set_data(uniform, data);
    }
}

void Shader_set_uniformBuffer(const Shader* shader, const char* alias, void* data) {
    /* Set the value of a Shader's Uniform by the variable name. */
    UniformBuffer* buffer;
    Shader_get_uniformBuffer(shader, alias, &buffer);
    UniformBuffer_set(buffer, alias, data);
}

void Shader_use(const Shader* shader) {

    glUseProgram(shader->Program);
       
    // for each non-buffer uniform, upload it to the GPU.
    for (uint64_t i = 0; i < Shader_get_uniform_count(shader); i++) {
        Uniform* uniform = HashTable_array_at(Uniform, shader->Uniforms, i);
        if (uniform != NULL) {
            upload_from_gl_type(i, uniform->Type, uniform->Elements, Uniform_get_data(void, uniform));
        }
    }
    

    // Set the active texture for each texture in the material.
    //for (uint16_t i = 0; i < material->TexturesUsed; i++) {
    //    if (material->Textures[i] != nullptr) {
    //        glActiveTexture(GL_TEXTURE0 + i);
    //        glBindTexture(GL_TEXTURE_2D, material->Textures[i]->ID);
    //    }
    //}
}

GLint internal_Program_buffer_count(const GLuint program) {
    
    GLint* params = (GLint*)malloc(sizeof(GLint));

    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, params);

    //glGetProgramiv(program, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, params);

    if (params == NULL) {
        // something has gone very wrong and the shader has no uniforms.
        assert(false);
    }

    GLint uniformCount = *params;
    free(params);

    return uniformCount;
}

GLint internal_Program_uniform_count(const GLuint program) {
    // Get the number of uniforms.

    GLint* params = (GLint*)malloc(sizeof(GLint));
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, params);

    if (params == NULL) {
        // something has gone very wrong and the shader has no uniforms.
        assert(false);
    }

    GLint uniformCount = *params;
    free(params);
    
    return uniformCount;
}


void internal_Program_buffer_parse(const GLuint program, HashTable* table) {
    //  Function to parse out uniform blocks from a shader. inserts them into the provided table, as well as the global table.
    //
    //

    assert(StorageBufferTable != NULL);

    GLint bufferCount = internal_Program_buffer_count(program);
    GLint binding;
    GLint indicies;
    GLint* uniformIndicies;
    GLint aliasLength;
    GLint size;
    char* alias;

    char* buffer = (char*)malloc(MAX_ALIAS_SIZE);
    assert(buffer != NULL);

    for (GLint i = 0; i < bufferCount; i++) {

        // get the name of the uniform buffer:
        //glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &aliasLength);
        
        glGetActiveUniformBlockName(program, i, MAX_ALIAS_SIZE, &aliasLength, buffer);
        alias = (char*)malloc(aliasLength + 1);
        assert(alias != NULL);
        memcpy(alias, buffer, aliasLength + 1);

        UniformBuffer* newBuffer;
        HashTable_find(StorageBufferTable, alias, &newBuffer);

        // if it already exists, insert that one instead.
        if (newBuffer) {
            newBuffer->References++;
            HashTable_insert(table, newBuffer->Alias, newBuffer);
            free(alias);
            continue;
        }

        // Otherwise, we need to fill out the other values and create a new buffer.
        glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_BINDING, &binding);
        glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
        glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &indicies);

        newBuffer = (UniformBuffer*)calloc(1, sizeof(UniformBuffer) + size);
        assert(newBuffer != NULL);


        newBuffer->Uniforms = HashTable_create(Uniform, indicies);
        uniformIndicies = (GLint*)calloc(indicies, sizeof(GLint));
        assert(uniformIndicies != NULL);
        glGetActiveUniformBlockiv(program, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uniformIndicies);

        internal_Program_buffer_uniform_parse(program, indicies, uniformIndicies, newBuffer);

        free(uniformIndicies);

        newBuffer->Alias = alias;
        newBuffer->AliasLength = aliasLength;
        newBuffer->UniformType = UNIFORM_TYPE_BUFFER;
        newBuffer->Size = size;
        newBuffer->BindingIndex = binding;
        newBuffer->References = 1;

        glGenBuffers(1, &(newBuffer->BufferObject));
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, newBuffer->BufferObject);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, newBuffer->BindingIndex, newBuffer->BufferObject);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, GL_NONE);

        // Insert the buffer into the both the storage buffer table and the local table.
        HashTable_insert(StorageBufferTable, alias, newBuffer);
        HashTable_insert(table, alias, newBuffer);
    }
}

static void internal_Program_buffer_uniform_parse(const GLuint program, const uint16_t uniformCount, const GLint* indicies, UniformBuffer* uniformBuffer) {

    GLint* blockOffsetParams = (GLint*)malloc(uniformCount * sizeof(GLint));
    assert(blockOffsetParams != NULL);

    char* buffer = (char*)malloc(MAX_ALIAS_SIZE);
    assert(buffer != NULL);
    
    glGetActiveUniformsiv(program, uniformCount, indicies, GL_UNIFORM_OFFSET, blockOffsetParams);

    for (uint16_t i = 0; i < uniformCount; i++) {
        
        GLsizei length;
        GLint elements;
        GLenum type;
        uint16_t CurrentOffset = 0;

        glGetActiveUniform(program, indicies[i], MAX_ALIAS_SIZE, &length, &elements, &type, buffer);
       
        char* alias = (char*)malloc(length + 1);
        assert(alias != NULL);
        memcpy(alias, buffer, length + 1);

        UniformInformation info = {alias, length + 1, UNIFORM_TYPE_BUFFER_ITEM, indicies[i], type, elements, blockOffsetParams[i]};
        HashTable_insert(uniformBuffer->Uniforms, alias, internal_UniformBuffer_item_create(&info, UniformBuffer_get_shared(uniformBuffer)));
    }

    free(blockOffsetParams);
    free(buffer);
}

void internal_Program_uniform_parse(const GLuint program, HashTable* table) {
    // Function to parse out uniforms from a shader. Inserts the uniforms into the provided table.
    //
    //
    
    GLint uniformCount = internal_Program_uniform_count(program);
    
    GLint* blockOffsetParams = (GLint*)malloc(uniformCount * sizeof(GLint));
    GLuint* indicies = (GLint*)malloc(uniformCount * sizeof(GLint));
    Uniform** uniforms = (Uniform**)calloc(uniformCount, sizeof(Uniform*));

    assert(blockOffsetParams != NULL);
    assert(indicies != NULL);
    assert(uniforms != NULL);

    for(GLuint i = 0; i < uniformCount; i++ ) {
        indicies[i] = i;
    }

    glGetActiveUniformsiv(program, uniformCount, indicies, GL_UNIFORM_OFFSET, blockOffsetParams);
    free(indicies);

    char* buffer = (char*)malloc(MAX_ALIAS_SIZE);
    assert(buffer != NULL);

    for (GLint i = 0; i < uniformCount; i++) {

        if (blockOffsetParams[i] != -1) {
            continue;
        }

        GLsizei length;
        GLint elements;
        GLenum type;

        glGetActiveUniform(program, i, MAX_ALIAS_SIZE, &length, &elements, &type, buffer);

        char* alias = (char*)malloc(length + 1);
        assert(alias != NULL);
        memcpy(alias, buffer, length + 1);

        UniformInformation info = { alias, length + 1, UNIFORM_TYPE_SINGLE, i, type, elements, -1 };
        HashTable_insert(table, alias, internal_Uniform_create(&info));
    }

    free(blockOffsetParams);
    free(buffer);
}
