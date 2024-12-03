#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "cStringUtilities.h"
#include "gl_types.h"
#include "gl_shader_uniform.h"

#define MAX_ALIAS_SIZE 512

static uint16_t BufferCount = 0;


UniformBuffer* UniformBuffer_create(const char* alias, const uint64_t size) {
    // Returns a pointer to a UniformBuffer object.
    //
    //
    
    UniformBuffer* newBuffer = (UniformBuffer*)malloc(sizeof(UniformBuffer));
    assert(newBuffer != NULL);
    
    //maybe move this alias creation stuff to a function in cStringUtilities.
    char* aliasEnd = FindBufferEnd(alias);
    uint64_t aliasSize = aliasEnd - alias + 1;
     
    char* bufferAlias = (char*)malloc(aliasSize);
    assert(bufferAlias != NULL);

    memcpy(bufferAlias, alias, aliasSize); 
    newBuffer->Alias = bufferAlias;
    newBuffer->AliasEnd = bufferAlias + aliasSize - 1;

    newBuffer->Size = size;
    newBuffer->BindingIndex = BufferCount++;

    glGenBuffers(1, &(newBuffer->BufferObject));
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, newBuffer->BufferObject);
    glBufferData(GL_SHADER_STORAGE_BUFFER, newBuffer->BufferObject, NULL, GL_STATIC_DRAW);          // write null into the buffer to set its size.
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, newBuffer->BindingIndex, newBuffer->BufferObject);   // set the binding index
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, GL_NONE);

    return newBuffer;
}

void UniformBuffer_destroy(UniformBuffer** buffer) {
    glDeleteBuffers(1, &((*buffer)->BufferObject));
    free((*buffer)->Alias);
    free(*buffer);
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

Uniform* internal_Uniform_create(const GLenum Type, const GLuint elements, const char* name, const int length) {
    // Internal function to initialize a Uniform* 
    
    assert(elements != 0);

    // Get the byte size of the type.
    int size = size_from_gl_type(Type);

    // allocate enough space for the Uniform header + the space required to store it's data. 
    Uniform* newUniform = (Uniform*)calloc(1, sizeof(Uniform) + (size * elements));
    assert(newUniform != NULL);
    newUniform->Size = size;
    newUniform->Elements = elements;
    newUniform->Type = Type;

    // allocate the buffer for the alias including the null terminator.
    newUniform->Alias = (char*)malloc(length + 1);
    assert(newUniform->Alias != NULL);

    memcpy(newUniform->Alias, name, length + 1);
    newUniform->AliasEnd = newUniform->Alias + length;

    return newUniform;
}


Shader* Shader_create(const GLuint program, const char* alias) {
    /* create a new shader, populate the fields and return a pointer to it. */

    GLint uniformCount = internal_Program_uniform_count(program);
    size_t UniformArraySize = uniformCount * sizeof(Uniform*);
    size_t UniformLookupSize = uniformCount * sizeof(uint64_t);
    size_t shaderAlocationSize = sizeof(Shader) + UniformArraySize + UniformLookupSize;

    // Just in case there's any alignment issues, ensure the block is padded to be exactly a multiple of four.
    shaderAlocationSize += shaderAlocationSize % 4;

    // Allocate the shader and it's buffers.
    Shader* shader = (Shader*)malloc(shaderAlocationSize);
    assert(shader != NULL);

    // Store the Uniforms and Lookup as direct offsets into the same memory block.
    shader->Uniforms = (Uniform**)((char*)shader + sizeof(Shader));
    shader->Lookup = (uint64_t*)((char*)shader + sizeof(Shader) + UniformArraySize);

    char* buffer = (char*)malloc(MAX_ALIAS_SIZE);

    for(GLint i = 0; i < uniformCount; i++) {

        GLsizei length;                       
        GLint elements;
        GLenum type;

        glGetActiveUniform(program, i, MAX_ALIAS_SIZE, &length, &elements, &type, buffer);
        
        // skip any invalid elements.
        if (size_from_gl_type(type) == -1) {
            continue;
        }

        Uniform* newUniform = internal_Uniform_create(type, elements, buffer, length);
        shader->Uniforms[i] = newUniform;
    }

    free(buffer);

    // Generate the lookup for the data.

    memset(shader->Lookup, 0xFF, uniformCount * sizeof(uint64_t));
    // ^^^ 0xFF is a safe assumption because the maximum uniforms OpenGL can handle is 1024.
    
    for(GLint i = 0; i < uniformCount; i++) {
        Uniform* uniform = shader->Uniforms[i];
        uint64_t hash = fnvHash64(uniform->Alias, uniform->AliasEnd) % uniformCount;
        uint16_t originalHash = hash;

        // linearly probe for an open spot.
        while (shader->Lookup[hash] < uniformCount) {
            hash++;
            hash %= uniformCount;

            if(hash == originalHash) {
                assert(false);
            }
        }

        // set the lookup value so the array of Uniforms can be kept in order.
        // When accessing uniforms by alias, do the hash table lookup to get the index of the uniform.
        shader->Lookup[hash] = i;
    }

    char* aliasEnd = FindBufferEnd(alias);
    uint64_t aliasLength = aliasEnd - alias + 1;

    char* shaderName = (char*)malloc(aliasLength);
    assert(shaderName != NULL);

    memcpy(shaderName, alias, aliasLength);

    shader->Program = program;
    shader->Alias = shaderName;
    shader->AliasEnd = shaderName + aliasLength - 1;
    shader->Program = program;
    shader->UniformCount = uniformCount;

    return shader;

}

void Shader_destroy(Shader** shader){
    /* Free a shader allocated with CreateShader. */
    
    glDeleteProgram((*shader)->Program);
    (*shader)->Program = GL_NONE;

    for(GLuint i = 0; i < (*shader)->UniformCount; i++) {
        Uniform* uniform = (*shader)->Uniforms[i];
        free(uniform->Alias);
        free(uniform);
    }

    free((*shader)->Alias);
    free((*shader));
    *shader = NULL;
}


void Shader_get_uniform(const Shader* shader, const char* alias, Uniform** outVal) {
    /* Get the reference to a Shader's Uniform by the variable name. */

    char* aliasEnd = FindBufferEnd(alias);
    uint64_t hash = fnvHash64(alias, aliasEnd) % shader->UniformCount;
    uint16_t originalHash = hash;

    // iterate through the lookup table until a match has been found.
    while (strcmp(shader->Uniforms[shader->Lookup[hash]]->Alias, alias) != 0) {
        hash++;
        hash %= shader->UniformCount;

        if(hash == originalHash) {
            *outVal = NULL;
            return;
        }
    }

    *outVal = shader->Uniforms[shader->Lookup[hash]];
}

void Shader_set_uniform(const Shader* shader, const char* alias, void* data) {
    /* Set the value of a Shader's Uniform by the variable name. */

    Uniform* uniform;
    Shader_get_uniform(shader, alias, &uniform);

    if (uniform != NULL) {
        Uniform_set_data(uniform, data);
    }
}

void Shader_use(const Shader* shader) {

    glUseProgram(shader->Program);
       
    for (GLint i = 0; i < shader->UniformCount; i++) {
        upload_form_gl_type(i, shader->Uniforms[i]->Type, shader->Uniforms[i]->Elements, Uniform_get_data(void, shader->Uniforms[i]));
    }


    // Set the active texture for each texture in the material.
    //for (uint16_t i = 0; i < material->TexturesUsed; i++) {
    //    if (material->Textures[i] != nullptr) {
    //        glActiveTexture(GL_TEXTURE0 + i);
    //        glBindTexture(GL_TEXTURE_2D, material->Textures[i]->ID);
    //    }
    //}
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
    char* buffer = (char*)malloc(MAX_ALIAS_SIZE);

    for (GLint i = 0; i < *params; i++) {
        GLsizei length;
        GLint elements;
        GLenum type;

        glGetActiveUniform(program, i, MAX_ALIAS_SIZE, &length, &elements, &type, buffer);

        // if an invalid uniform was found, decrement the count.
        //if (size_from_gl_type(type) == -1) {
        //    uniformCount--;
        //}
    }

    free(params);
    free(buffer);

    return uniformCount;
}
