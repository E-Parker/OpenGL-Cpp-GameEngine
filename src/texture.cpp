#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <cassert>
#include <cstring>
#include <iostream>

#include "hashTable.h"
#include "texture.h"

static HashTable<Texture> TextureTable(512);

bool TextureManager::FindTexture(const char* alias, Texture*& outValue) {
    return TextureTable.Find(alias, outValue);
}

void TextureManager::InternalUploadTexture(Texture* texture, uint8_t* data, GLenum internalFormat, GLenum format, GLenum uploadType = GL_TEXTURE_2D) {
    
    if (texture->ID == GL_NONE) {
        glGenTextures(1, &texture->ID);
    }

    glBindTexture(texture->type, texture->ID);
    glTextureParameteri(texture->ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture->ID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(texture->ID, GL_TEXTURE_MIN_FILTER, texture->filterType);
    glTextureParameteri(texture->ID, GL_TEXTURE_MAG_FILTER, texture->filterType);
    glTexImage2D(uploadType, 0, internalFormat, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, data);
}

void TextureManager::InternalUploadTextureMimmap(Texture* texture, uint8_t* data, GLenum internalFormat, GLenum format, GLenum uploadType = GL_TEXTURE_2D) {
    
    if (texture->ID == GL_NONE) {
        glGenTextures(1, &texture->ID);
    }

    glBindTexture(texture->type, texture->ID);
    glTextureParameteri(texture->ID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture->ID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(texture->ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture->ID, GL_TEXTURE_MAG_FILTER, texture->filterType);
    glTexImage2D(uploadType, 0, internalFormat, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(texture->type);
}

void TextureManager::InternalDeleteTexture(Texture* texture) {
    if (--texture->references == 0) {
        if (texture->ID != GL_NONE) {
            glDeleteTextures(1, &(texture->ID));
        }
        texture->ID = GL_NONE;
        TextureTable.Delete(texture->alias);
    }
}


void TextureManager::InternalCreateTexture(Texture* texture, const bool isManaged, const char* alias, const GLenum internalFormat, const GLenum format, uint8_t* data, bool useMipmap) {
    /* Internal function for creating managed textures. */

    if (useMipmap) {
        TextureManager::InternalUploadTextureMimmap(texture, data, internalFormat, format);
    }
    else {
        TextureManager::InternalUploadTexture(texture, data, internalFormat, format);
    }

    texture->alias = TextureTable.Insert(alias, texture, isManaged);
    texture->references++;

}


void TextureManager::CreateRawTexture(const char* path, Texture* texture, GLenum internalFormat, GLenum format, bool flipVertical, bool flipHorizontal, bool useMipmaps, int filterType) {
    /* This function creates a new, unmanaged texture from the file path and the alias. if the texture already exists in memory, the returned value will be that one. */

    texture->filterType = filterType;

    stbi_set_flip_vertically_on_load(true);
    uint8_t* data = stbi_load(path, &texture->width, &texture->height, &texture->channels, 0);

    if (data == nullptr) {
        std::cout << "Error creating raw Texture from: \"" << path << "\". The texture will be discarded." << std::endl;
        return;
    }

    // Create the managed texture and upload the texture to the GPU.
    TextureManager::InternalCreateTexture(texture, true, path, internalFormat, format, data, useMipmaps);

    // Free the data generated by stb_image.
    stbi_image_free(data);

}


Texture* CreateTexture(const char* path, const char* alias, GLenum internalFormat, GLenum format, bool flipVertical, bool flipHorizontal, bool useMipmaps, int filterType) {
    /* This function creates a new texture from the file path and the alias. if the texture already exists in memory, the returned value will be that one.
    This will hopefully save delectably scrumptious graphics memory mmmhh. If an alias is not provided, the texture will use it's path as an alias. */

    char* aliasUsed;
    Texture* texture = nullptr;

    // If the alias is empty, use the path instead.
    if (!strcmp(alias, "")) { aliasUsed = const_cast<char*>(path); }
    else { aliasUsed = const_cast<char*>(alias); }

    // try to find the texture in the table.
    TextureTable.Find(aliasUsed, texture);

    // if the texture doesn't already exist, make a new one, and return that instead.
    if (texture != nullptr) {
        return texture;
    }

    texture = new Texture();
    texture->filterType = filterType;

    stbi_set_flip_vertically_on_load(true);
    uint8_t* data = stbi_load(path, &texture->width, &texture->height, &texture->channels, 0);

    if (data == nullptr) {
        std::cout << "Error creating Texture: \"" << aliasUsed << "\" From: \"" << path << "\". The texture will be discarded." << std::endl;
        return nullptr;
    }

    // Create the managed texture and upload the texture to the GPU.
    TextureManager::InternalCreateTexture(texture, false, aliasUsed, internalFormat, format, data, useMipmaps);

    // Free the data generated by stb_image.
    stbi_image_free(data);

    return texture;
}


Texture* CreateCubemapTexture(const char* texturePaths[6], const char* alias, GLenum internalFormat, GLenum format, bool flipVertical, bool flipHorizontal, bool useMipmaps, int filterType) {

    Texture* texture = nullptr;
    TextureTable.Find(alias, texture);

    // if the texture doesn't already exist, make a new one, and return that instead.
    if (texture != nullptr) {
        return texture;
    }
     
    texture = new Texture();
    texture->filterType = filterType;
    texture->type = GL_TEXTURE_CUBE_MAP;

    for (uint16_t i = 0; i < 6; i++) {

        stbi_set_flip_vertically_on_load(true);
        uint8_t* data = stbi_load(texturePaths[i], &texture->width, &texture->height, &texture->channels, 0);
        
        if (data == nullptr) {
            std::cout << "Error creating Cube Map Texture: \"" << alias << "\" At index, " << i << ", From: \"" << texturePaths[i] << "\". The texture will be discarded." << std::endl;
            return nullptr;
        }

        TextureManager::InternalUploadTexture(texture, data, internalFormat, format, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
        stbi_image_free(data);
    }

    return nullptr;
}


void DeleteTexture(const char* alias) {
    /* This function manages deleting textures from the table. The texture will be deleted from graphics memory if it isn't referenced anywhere. */

    Texture* texture = nullptr;

    if (alias == nullptr) {
        return;
    }

    TextureTable.Find(alias, texture);

    if (texture == nullptr) {
        std::cout << "Error deleting Texture: \"" << alias << "\". No Texture with that name found. " << std::endl;
        return;
    }

    TextureManager::InternalDeleteTexture(texture);
}


void DereferenceTextures() {
    /* Call this function at the end of your program to ensure all tracked textures are properly cleaned up. */

    // Iterate through all the positions in the hash table.
    for (uint64_t i = 0; i < TextureTable.Size; i++) {

        // Check if there is a value stored here:
        if (TextureTable.Array[i].Key == nullptr) {
            continue;
        }

        Texture* texture = TextureTable.Array[i].Value;

        // Forcefully clear the memory for all textures marked as managed.
        if (TextureTable.Array[i].isManaged) {
            std::cout << "Texture Manager: Freeing texture, \"" << texture->alias << "\"." << std::endl;
            texture->references = 1;
            TextureManager::InternalDeleteTexture(texture);
        }
    }
}
