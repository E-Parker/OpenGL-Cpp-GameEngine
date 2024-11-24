#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "shaderUniform.h"

#define MAX_ALIAS_SIZE 512

// I had to redefine these here because c doesn't know what const_cast<>() is.

uint64_t fnvHash64(const char* buffer, const char* const bufferEnd) {
    /* implementation of the fnv64 hashing function, created by Glenn Fowler, Landon Curt Noll,
    and Kiem-Phong Vo. I used fixed-width integers here for maximum portability. */

    const uint64_t Prime = 0x00000100000001B3;
    uint64_t Hash = 0xCBF29CE484222325;

    char* bufferIter = (char*)buffer;

    // Iterate from the buffer start address to the buffer end address.
    for (; bufferIter < bufferEnd; bufferIter++) {
        //XOR the current hash with the current character, then multiply by an arbitrary prime number.
        Hash = (Hash ^ (*bufferIter)) * Prime;
    }
    return Hash;
}


char* FindBufferEnd(const char* buffer) {

    char* bufferEnd = (char*)buffer;

    // Assume the key is a c_string, iterate through to find the null terminator.
    for (uint16_t i = 0; i < 0xFFFF; i++) {
        if (*bufferEnd == '\0') {
            break;
        }
        bufferEnd++;
    }

    return bufferEnd;
}


Shader* CreateShader(GLuint program, const char* alias) {
    /* create a new shader, populate the fields and return a pointer to it. */

    Shader* shader = (Shader*)malloc(sizeof(Shader));

    // Get the number of uniforms.
    GLint* params = NULL;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, params);
    GLint uniformCount = *params;
    free(params);

    // Generate array for the uniforms.
    shader->Uniforms = (Uniform*)malloc(uniformCount * sizeof(Uniform));
    uint16_t dataBlockSize = 0;

    // Get the uniforms.
    for(GLint i = 0; i < uniformCount; i++) {
        
        Uniform* uniform = &shader->Uniforms[i];        // Get the current uniform.
        char* buffer = (char*)malloc(MAX_ALIAS_SIZE);   // Set up buffer for the uniform's name.
        GLsizei actualLength;                       

        glGetActiveUniform(program, i, MAX_ALIAS_SIZE, &actualLength, &uniform->Size, &uniform->Type, buffer);
        actualLength++; // increment by 1 to account for the null terminator.

        shader->Uniforms[i].Alias = (char*)malloc(actualLength);    // allocate the buffer for the alias including the null terminator.
        memcpy(uniform->Alias, &buffer, actualLength);              // Copy the data to the alias
        uniform->AliasEnd = uniform->Alias + actualLength;          // Set a reference to the end of the string.
        dataBlockSize += uniform->Size;                             // Update the block size so we allocate enough data for each uniform.
    }

    // Generate the data block of the uniforms.
    //
    // the idea here is to store all of the uniforms in one homogeneous block so it can be uploaded 
    // in one pass. Each uniform's Data pointer just stores an offset into the block.
    //
    shader->Data = malloc(dataBlockSize);
    uint16_t offset = 0;

    for(GLint i = 0; i < uniformCount; i++) {
        Uniform* uniform = &shader->Uniforms[i];
        uniform->Data = (uint8_t*)shader->Data + offset;
        offset += uniform->Size;
        
    }

    // Generate the lookup for the data.
    shader->Lookup = (uint64_t*)malloc(uniformCount * sizeof(uint64_t));
    memset(shader->Lookup, 0xFF, uniformCount * sizeof(uint64_t));
    // ^^^ 0xFF is a safe assumption because the maximum uniforms OpenGL can handle is 1024.
    
    for(GLint i = 0; i < uniformCount; i++) {
        Uniform* uniform = &shader->Uniforms[i];
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
    char* shaderName = (char*)malloc(alias - aliasEnd);
    memcpy(shaderName, alias, alias - aliasEnd);

    shader->Program = program;
    shader->Alias = shaderName;
    shader->Program = program;

    return shader;

}


void FreeShader(Shader** shader){
    /* Free a shader allocated with CreateShader. */
    
    glDeleteProgram((*shader)->Program);
    (*shader)->Program = GL_NONE;

    for(GLuint i = 0; i < (*shader)->UniformCount; i++) {
        free((*shader)->Uniforms[i].Alias);
    }

    free((*shader)->Uniforms);
    free((*shader)->Data);
    free((*shader)->Lookup);
    free((*shader)->Alias);
    free((*shader));
    *shader = NULL;
}


void GetUniform(const Shader* shader, const char* alias, Uniform** outVal) {
    /* Get the reference to a Shader's Uniform by the variable name. */

    char* aliasEnd = FindBufferEnd(alias);
    uint64_t hash = fnvHash64(alias, aliasEnd) % shader->UniformCount;
    uint16_t originalHash = hash;

    // iterate through the lookup table until a match has been found.
    while (strcmp(shader->Uniforms[shader->Lookup[hash]].Alias, alias) != 0) {
        hash++;
        hash %= shader->UniformCount;

        if(hash == originalHash) {
            return;
            outVal = NULL;
        }
    }

    *outVal = &shader->Uniforms[shader->Lookup[hash]];
}

