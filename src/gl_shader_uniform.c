#include <glad/glad.h>
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

UniformBuffer* UniformBuffer_create(const UniformInformation* info, const uint64_t size) {
    // Returns a pointer to a UniformBuffer object.
    //
    //

    if (!info->isBuffer) {
        // Cannot create a uniformBuffer from a uniform!!
        return NULL;
    }

    assert(StorageBufferTable != NULL);

    // Attempt to locate the storage buffer in the HashTable. if it already exists and is bound, return it. 
    // Otherwise generate a new buffer.

    UniformBuffer* newBuffer;
    HashTable_find(StorageBufferTable, info->Alias, &newBuffer);

    if (newBuffer != NULL) {
        return newBuffer;
    }
    
    newBuffer = (UniformBuffer*)malloc(sizeof(UniformBuffer));
    assert(newBuffer != NULL);
    
 
    newBuffer->Alias = info->Alias;
    newBuffer->AliasEnd = info->AliasEnd;
    newBuffer->isBuffer = true;
    newBuffer->Size = size;
    newBuffer->BindingIndex = BufferCount++;

    glGenBuffers(1, &(newBuffer->BufferObject));
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, newBuffer->BufferObject);
    glBufferData(GL_SHADER_STORAGE_BUFFER, newBuffer->BufferObject, NULL, GL_STATIC_DRAW);          // write null into the buffer to initialize it.
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, newBuffer->BindingIndex, newBuffer->BufferObject);   // set the binding index. This allows for multiple shaders to access the same region.
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, GL_NONE);
    
    // Insert the storage buffer into the table for retrieval upon recurrence. 
    HashTable_insert(StorageBufferTable, info->Alias, newBuffer);

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

void UniformBuffer_get(const char* alias, UniformBuffer** outVal) {
    // Simple wrapper function to access the StorageBufferTable.
    assert(StorageBufferTable);
    HashTable_find(StorageBufferTable, alias, outVal);
}

Uniform* internal_Uniform_create(const UniformInformation* info) {
    // Internal function to initialize a Uniform* 
    
    if (info->isBuffer) {
        // Cannot create a uniform from a uniform buffer!!
        return NULL;
    }

    assert(info->Elements != 0);

    // Get the byte size of the type.
    int size = size_from_gl_type(info->Type);

    // allocate enough space for the Uniform header + the space required to store it's data. 
    Uniform* newUniform = (Uniform*)calloc(1, sizeof(Uniform) + (size * info->Elements));
    assert(newUniform != NULL);

    newUniform->isBuffer = false;
    newUniform->Alias = info->Alias;
    newUniform->AliasEnd = info->AliasEnd;
    newUniform->Size = size;
    newUniform->Elements = info->Elements;
    newUniform->Type = info->Type;

    return newUniform;
}

Shader* Shader_create(const GLuint program, const char* alias) {
    /* create a new shader, populate the fields and return a pointer to it. */

    GLint uniformCount = internal_Program_uniform_count(program);
    UniformInformation* uniforms = internal_Program_uniform_parse(program);

    // Something went wrong and padding needs to be added to one of these.
    assert(sizeof(Uniform) == sizeof(UniformBuffer));
    
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

    for(GLint i = 0; i < uniformCount; i++) {
        switch(uniforms[i].isBuffer) {
        case true: shader->Uniforms[i] = UniformBuffer_create(&uniforms[i], 1); break;
        case false: shader->Uniforms[i] = internal_Uniform_create(&uniforms[i]); break;
        default: shader->Uniforms[i] = NULL; break;
        }
    }

    // Generate the lookup for the data.
    memset(shader->Lookup, 0xFF, uniformCount * sizeof(uint64_t));
    // ^^^ 0xFF is a safe assumption because the maximum uniforms OpenGL can handle is 1024.
    
    for(GLint i = 0; i < uniformCount; i++) {
        Uniform* uniform = shader->Uniforms[i];
        
        if (!uniform) {
            continue;
        }

        // Shut up MSVC!!! oh my god! its literally impossible for the uniform to be NULL! FUCK OFF!
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

        // uniform buffers are handled externally.
        if (!uniform->isBuffer) {
            free(uniform->Alias);
            free(uniform);
        }
    }

    free((*shader)->Alias);
    free((*shader));
    *shader = NULL;
}


void internal_Shader_get_uniform(const Shader* shader, const char* alias, void** outVal) {
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

void Shader_get_uniform(const Shader* shader, const char* alias, Uniform** outVal) {
    internal_Shader_get_uniform(shader, alias, (void**)outVal);
}

void Shader_get_uniformBuffer(const Shader* shader, const char* alias, UniformBuffer** outVal) {
    internal_Shader_get_uniform(shader, alias, (void**)outVal);
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

    UniformBuffer* uniform;
    Shader_get_uniformBuffer(shader, alias, &uniform);

    if (uniform != NULL) {
        UniformBuffer_set_all(uniform, data);
    }
}

void Shader_use(const Shader* shader) {

    glUseProgram(shader->Program);
       
    // for each non-buffer uniform, upload it to the GPU.
    for (GLint i = 0; i < shader->UniformCount; i++) {
        if (!shader->Uniforms[i]->isBuffer) {
            upload_form_gl_type(i, shader->Uniforms[i]->Type, shader->Uniforms[i]->Elements, Uniform_get_data(void, shader->Uniforms[i]));
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

UniformInformation* internal_Program_uniform_parse(const GLuint program) {
    // Function to parse out uniforms from a shader. Returns an array of struts containing uniform information. 
    // Any uniform type should be able to be created from this.
    //

    GLint uniformCount = internal_Program_uniform_count(program);
    UniformInformation* uniforms = (UniformInformation*)calloc(uniformCount, sizeof(UniformInformation));
    GLint* indicies = (GLint*)malloc(uniformCount * sizeof(GLint));
    GLint* blockIndexParams = (GLint*)malloc(uniformCount * sizeof(GLint));
    GLint* blockOffsetParams = (GLint*)malloc(uniformCount * sizeof(GLint));

    assert(indicies != NULL);
    assert(uniforms != NULL);
    assert(blockIndexParams != NULL);
    assert(blockOffsetParams != NULL);

    for (GLint i = 0; i < uniformCount; i++) {
        indicies[i] = i;
    }

    glGetActiveUniformsiv(program, uniformCount, indicies, GL_UNIFORM_BLOCK_INDEX, blockIndexParams);

    char* buffer = (char*)malloc(MAX_ALIAS_SIZE);
    assert(buffer != NULL);

    for (GLint i = 0; i < uniformCount; i++) {

        GLsizei length;
        GLint elements;
        GLenum type;

        glGetActiveUniform(program, i, MAX_ALIAS_SIZE, &length, &elements, &type, buffer);

        char* alias = (char*)malloc(length + 1);
        assert(alias != NULL);
        memcpy(alias, buffer, length + 1);

        uniforms[i].Alias = alias;
        uniforms[i].AliasEnd = alias + length;
        uniforms[i].Location = i;
        uniforms[i].Elements = elements;
        uniforms[i].BlockIndex = blockIndexParams[i];
        uniforms[i].BlockOffset = blockOffsetParams[i];
    }

    free(buffer);
    free(indicies);
    free(blockIndexParams);
    free(blockOffsetParams);

    return uniforms;
}
