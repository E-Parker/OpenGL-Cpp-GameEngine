#pragma once

#include <cassert>

template <typename T> class HashTable;

typedef struct Texture {
    // Struct to hold graphics data for a texture. Avoid using this manually since the system cannot track it.

    char* alias;
    GLuint ID = GL_NONE;
    GLenum type = GL_TEXTURE_2D;
    uint64_t references = 0;
    int width = 0;
    int height = 0;
    int filterType = 0;
    int channels = 0;

} Texture;


namespace TextureManager {
    void InternalUploadTexture(Texture* texture, uint8_t* data, GLenum internalFormat, GLenum format, GLenum uploadType);
    void InternalUploadTextureMimmap(Texture* texture, uint8_t* data, GLenum internalFormat, GLenum format, GLenum uploadType);
    void InternalDeleteTexture(Texture* texture);
    void InternalCreateTexture(Texture* texture, const bool isManaged, const char* alias, const GLenum internalFormat, const GLenum format, uint8_t* data, const bool useMipmaps);
    void CreateRawTexture(const char* path, Texture* texture, GLenum internalFormat = GL_RGBA, GLenum format = GL_RGBA, bool flipVertical = false, bool flipHorizontal = false, bool useMipmaps = true, int filterType = GL_LINEAR);
    
    bool FindTexture(const char* alias, Texture*& outValue);

}

void DereferenceTextures();
void DeleteTexture(const char* alias);
Texture* CreateTexture(const char* path, const char* alias = "", GLenum internalFormat = GL_RGBA, GLenum format = GL_RGBA, bool flipVertical = false, bool flipHorizontal = false, bool useMipmaps = true, int filterType = GL_LINEAR);
Texture* CreateCubemapTexture(const char* texturePaths[6], const char* alias, GLenum internalFormat = GL_RGBA, GLenum format = GL_RGBA, bool flipVertical = false, bool flipHorizontal = false, bool useMipmaps = true, int filterType = GL_LINEAR);
