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


const int size_from_gl_type(const GLenum Type) {
    // I would like to not have a massive switch case but there isn't really a better way.
    // The API just does not have functionality for this since GLfloat and GLint are always assumed to be 
    // exactly the same as C / C++ standards. 
    switch (Type) {
    case GL_FLOAT: return sizeof(GLfloat);
    case GL_FLOAT_VEC2: return sizeof(GLfloat) * 2;
    case GL_FLOAT_VEC3: return sizeof(GLfloat) * 3;
    case GL_FLOAT_VEC4: return sizeof(GLfloat) * 4;
    case GL_DOUBLE: return sizeof(GLdouble);
    case GL_DOUBLE_VEC2: return sizeof(GLdouble) * 2;
    case GL_DOUBLE_VEC3: return sizeof(GLdouble) * 3;
    case GL_DOUBLE_VEC4: return sizeof(GLdouble) * 4;
    case GL_INT: return sizeof(GLint);
    case GL_INT_VEC2: return sizeof(GLint) * 2;
    case GL_INT_VEC3: return sizeof(GLint) * 3;
    case GL_INT_VEC4: return sizeof(GLint) * 4;
    case GL_UNSIGNED_INT: return sizeof(GLuint);
    case GL_UNSIGNED_INT_VEC2: return sizeof(GLuint) * 2;
    case GL_UNSIGNED_INT_VEC3: return sizeof(GLuint) * 3;
    case GL_UNSIGNED_INT_VEC4: return sizeof(GLuint) * 4;
    case GL_BOOL: return sizeof(GLboolean);
    case GL_BOOL_VEC2: return sizeof(GLboolean) * 2;
    case GL_BOOL_VEC3: return sizeof(GLboolean) * 3;
    case GL_BOOL_VEC4: return sizeof(GLboolean) * 4;
    case GL_FLOAT_MAT2: return sizeof(GLfloat) * 4;
    case GL_FLOAT_MAT2x3: return sizeof(GLfloat) * 6;
    case GL_FLOAT_MAT2x4: return sizeof(GLfloat) * 8;
    case GL_FLOAT_MAT3: return sizeof(GLfloat) * 9;
    case GL_FLOAT_MAT3x2: return sizeof(GLfloat) * 6;
    case GL_FLOAT_MAT3x4: return sizeof(GLfloat) * 12;
    case GL_FLOAT_MAT4: return sizeof(GLfloat) * 16;
    case GL_FLOAT_MAT4x2: return sizeof(GLfloat) * 6;
    case GL_FLOAT_MAT4x3: return sizeof(GLfloat) * 12;
    case GL_DOUBLE_MAT2: return sizeof(GLfloat) * 4;
    case GL_DOUBLE_MAT2x3: return sizeof(GLfloat) * 6;
    case GL_DOUBLE_MAT2x4: return sizeof(GLfloat) * 8;
    case GL_DOUBLE_MAT3: return sizeof(GLfloat) * 9;
    case GL_DOUBLE_MAT3x2: return sizeof(GLfloat) * 6;
    case GL_DOUBLE_MAT3x4: return sizeof(GLfloat) * 12;
    case GL_DOUBLE_MAT4: return sizeof(GLfloat) * 16;
    case GL_DOUBLE_MAT4x2: return sizeof(GLfloat) * 6;
    case GL_DOUBLE_MAT4x3: return sizeof(GLfloat) * 12;
    default: return 0;
    }
}


Uniform* init_uniform(const GLenum Type, const GLuint elements, const char* name, const int length) {
    // Internal function to initialize a Uniform* 
    
    assert(elements != 0);

    // Get the byte size of the type.
    int size = size_from_gl_type(Type);

    // allocate enough space for the Uniform header + the space required to store it's data. 
    Uniform* newUniform = (Uniform*)malloc(sizeof(Uniform) + (size * elements));
    assert(newUniform != NULL);

    // Set size and number of elements variables.
    newUniform->Size = size;
    newUniform->Elements = elements;

    // allocate the buffer for the alias including the null terminator.
    newUniform->Alias = (char*)malloc(length + 1);
    assert(newUniform->Alias != NULL);

    memcpy(newUniform->Alias, name, length + 1);
    newUniform->AliasEnd = newUniform->Alias + length + 1;

    // zero out the data section, if it exists.
    if (size != 0) {
        memset(uniform_data(newUniform), 0, uniform_size(newUniform));
    }

    return newUniform;
}


GLint UniformCount(const GLuint program) {
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
        if (size_from_gl_type(type) == -1) {
            uniformCount--;
        }
    }

    free(params);
    free(buffer);

    return uniformCount;
}


Shader* CreateShader(const GLuint program, const char* alias) {
    /* create a new shader, populate the fields and return a pointer to it. */

    GLint uniformCount = UniformCount(program);
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

        shader->Uniforms[i] = init_uniform(type, elements, buffer, length);
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
    shader->AliasEnd = aliasEnd;
    shader->Program = program;
    shader->UniformCount = uniformCount;

    return shader;

}


void FreeShader(Shader** shader){
    /* Free a shader allocated with CreateShader. */
    
    glDeleteProgram((*shader)->Program);
    (*shader)->Program = GL_NONE;

    for(GLuint i = 0; i < (*shader)->UniformCount; i++) {
        Uniform* uniform = (*shader)->Uniforms[i];
        free(uniform->Alias);
    }

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
    while (strcmp(shader->Uniforms[shader->Lookup[hash]]->Alias, alias) != 0) {
        hash++;
        hash %= shader->UniformCount;

        if(hash == originalHash) {
            outVal = NULL;
            return;
        }
    }

    *outVal = shader->Uniforms[shader->Lookup[hash]];
}

