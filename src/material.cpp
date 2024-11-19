#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>

#include "createShader.h"
#include "hashTable.h"
#include "material.h"
#include "texture.h"


Material::Material(const char* vertexProgramPath, const char* fragmentProgramPath, const uint16_t numberOfTextures, const GLenum cullFuncton, const GLenum depthFunction) {
    TexturesUsed = numberOfTextures;
    CullFunction = cullFuncton;
    DepthFunction = depthFunction;
    
    Textures = nullptr;
    
    if(TexturesUsed != 0) {
        Textures = new Texture*[TexturesUsed]{nullptr};
    }

    GLuint VertexProgram = GL_NONE;
    GLuint FragmentProgram = GL_NONE;

    // compile the shader programs
    char* vertSrc = CreateShader(&VertexProgram, GL_VERTEX_SHADER, vertexProgramPath);
    char* fragSrc = CreateShader(&FragmentProgram, GL_FRAGMENT_SHADER, fragmentProgramPath);
    Program = CreateProgram(VertexProgram, FragmentProgram);

    delete[] fragSrc;
    delete[] vertSrc;

}


Material::~Material() {
    
    if (this == nullptr) {
        std::cout << "Error deleting Material: Material is null!!! " << std::endl;
        return;
    }

    if (TexturesUsed != 0) {

        for (int i = 0; i < TexturesUsed; i++) {
            if (Textures[i] != nullptr) {
                DeleteTexture(Textures[i]->alias);
            }
        }
        delete[] Textures;
    }

    glDeleteProgram(Program);
    Program = GL_NONE;
}



typedef struct UniformObject {
    char* Type;
    char* Alias;
    void* Data;

    UniformObject(char* type, char* alias) : Type(type), Alias(alias) { }
    ~UniformObject() {
        delete[] Type;
        delete[] Alias;
        Type = nullptr;
        Alias = nullptr;
    }

} UniformObject;


std::vector<UniformObject*>* GetUniformsFromSource(const char* src) {
    // Parse the file line by line to find uniforms.    "uniform"

    std::stringstream stream(src);
    std::vector<UniformObject*>* uniforms = new std::vector<UniformObject*>();

    char lineBuffer[128];

    while (!stream.eof()) {

        stream.getline(lineBuffer, 128);       // Get the current line from the file.

        std::stringstream dataStream(lineBuffer);
        std::string segment;

        std::vector<std::string> segmentList;

        while (std::getline(dataStream, segment, ' ')) {
            segmentList.push_back(segment);
        }

        if (segmentList.size() < 3) {
            continue;
        }

        if (strcmp(segmentList[0].c_str(), "uniform") != 0) {
            continue;
        }

        char* type = new char[segmentList[1].length() + 1]; // Make room for the null terminator.
        memcpy(type, segmentList[1].c_str(), segmentList[1].length() + 1);

        char* alias = new char[segmentList[2].length()];    // Replace the ";" at the end of the line with the null terminator.
        memcpy(alias, segmentList[2].c_str(), segmentList[1].length());
        alias[segmentList[1].length()] = '\0';

        UniformObject* newObject = new UniformObject(type, alias);

        uniforms->push_back(newObject);
    }
    return uniforms;
}



void SetTextureFromPointer(const Material* material, Texture* texture, uint16_t index){
    /* Manually set a texture from a texture pointer. AVOID USING!!!
    The textures set this way will be UNAMANGED and must be freed MANUALLY. */

    if (material == nullptr) {
        std::cout << "Error setting Material Texture: Material is null!!! " << std::endl;
        return;
    }
    
    if (index >= material->TexturesUsed) {
        std::cout << "Warning: Material Texture index out of range. Discarding texture. " << std::endl;
        return;
    }

    if (texture == nullptr) {
        std::cout << "Error setting Material Texture: Texture must not be null." << std::endl;
        return;
    }

    // If a texture already exists in that slot, try to delete it.
    if (material->Textures[index] != nullptr) {
        DeleteTexture(material->Textures[index]->alias);
    }

    material->Textures[index] = texture;
    texture->references++;
}

void SetTextureFromAlias(const Material* material, const char* alias, uint16_t index) {
    /* Set a material's texture at the given index, by the texture's alias. */

    if (material == nullptr) {
        std::cout << "Error setting Material Texture: Material is null!!! " << std::endl;
        return;
    }

    if (index >= material->TexturesUsed) {
        std::cout << "Warning: Material Texture index out of range. Discarding texture. " << std::endl;
        return;
    }

    Texture* texture = nullptr;
    TextureManager::FindTexture(alias, texture);

    if (texture == nullptr) {
        std::cout << "Error setting Material Texture: \"" << alias << "\" At index: " << index << ". The texture could not be found." << std::endl;
        return;
    }

    // If a texture already exists in that slot, try to delete it.
    if (material->Textures[index] != nullptr) {
        DeleteTexture(material->Textures[index]->alias);
    }

    material->Textures[index] = texture;
    texture->references++;
}


void BindMaterial(const Material* material){
    /* Set up the material for rendering. */

    if (material == nullptr) {
        return;
    }

    // Set the shader program and get the uniform from the shader.
    glUseProgram(material->Program);
    glCullFace(material->CullFunction);
    glDepthFunc(material->DepthFunction);

    // Set the active texture for each texture in the material.
    for (uint16_t i = 0; i < material->TexturesUsed; i++) {
        if (material->Textures[i] != nullptr) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, material->Textures[i]->ID);
        }
    }
}

