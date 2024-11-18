#pragma once

#include <cassert>

template <typename T> class HashTable;

typedef struct Texture {
    // Struct to hold graphics data for a texture. Avoid using this manually since the system cannot track it.

    char* alias;
    GLuint ID = GL_NONE;

    uint64_t references;
    int width, height, filterType, channels;


    Texture(const Texture& texture) {
        // Textures should NEVER be copied!
        assert(false);
    }

    Texture() : alias(nullptr), ID(GL_NONE), filterType(GL_LINEAR), width(0), height(0), channels(0), references(0) { }
    ~Texture() { }

} Texture;


namespace TextureManager {
    void InternalUploadTexture(Texture* texture, uint8_t* data, GLenum internalFormat, GLenum format);
    void InternalUploadTextureMimmap(Texture* texture, uint8_t* data, GLenum internalFormat, GLenum format);
    void InternalDeleteTexture(Texture* texture);
    void InternalCreateTexture(Texture* texture, const bool isManaged, const char* alias, const GLenum internalFormat, const GLenum format, uint8_t* data, const bool useMipmaps);
    void CreateRawTexture(const char* path, Texture* texture, GLenum internalFormat = GL_RGBA, GLenum format = GL_RGBA, bool flipVertical = false, bool flipHorizontal = false, bool useMipmaps = true, int filterType = GL_LINEAR);
    
    bool FindTexture(const char* alias, Texture*& outValue);

}

void DereferenceTextures();
void DeleteTexture(const char* alias);
Texture* CreateTexture(const char* path, const char* alias = "", GLenum internalFormat = GL_RGBA, GLenum format = GL_RGBA, bool flipVertical = false, bool flipHorizontal = false, bool useMipmaps = true, int filterType = GL_LINEAR);

