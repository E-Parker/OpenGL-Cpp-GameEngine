#pragma once

typedef struct Texture {
    // Struct to hold graphics data for a texture. Avoid using this manually since the system cannot track it.

    char* alias;
    GLuint ID = GL_NONE;
        
    uint64_t references;
    int width, height, filterType, channels;
    

    inline Texture(const Texture& texture) {
        // Textures should NEVER be copied!
        assert(false);
    }

    inline Texture() : alias(nullptr), ID(GL_NONE), filterType(GL_LINEAR), width(0), height(0), channels(0), references(0) { }
    inline ~Texture() { }

} Texture;

class Material {

public:

    uint16_t TexturesUsed = 0;
    Texture** Textures;
    
    GLuint Program = GL_NONE;
    GLenum CullFunction = GL_BACK;
    GLenum DepthFunction = GL_LESS;

    // Constructors:
    Material(const GLuint vertexProgram, const GLuint fragmentProgram, const uint16_t numberOfTextures, const GLenum cullFuncton, const GLenum depthFunction);
    Material(const char* vertexProgramPath, const char* fragmentProgramPath, const uint16_t numberOfTextures, const GLenum cullFuncton, const GLenum depthFunction);

    ~Material();

    void SetTexture(Texture* texture, uint16_t index);
    void SetTexture(const char* alias, uint16_t index);
    void BindMaterial();

};

namespace TextureManager {
    void InternalUploadTexture(Texture* texture, uint8_t* data, GLenum internalFormat, GLenum format);
    void InternalUploadTextureMimmap(Texture* texture, uint8_t* data, GLenum internalFormat, GLenum format);
    void InternalDeleteTexture(Texture* texture);
    void InternalCreateTexture(Texture* texture, const bool isManaged, const char* alias, const GLenum internalFormat, const GLenum format, uint8_t* data, const bool useMipmaps);
    void CreateRawTexture(const char* path, Texture* texture, GLenum internalFormat = GL_RGBA, GLenum format = GL_RGBA, bool flipVertical = false, bool flipHorizontal = false, bool useMipmaps = true, int filterType = GL_LINEAR);

}
void DereferenceTextures();

void DeleteTexture(const char* alias);
Texture* CreateTexture(const char* path, const char* alias = "", GLenum internalFormat = GL_RGBA, GLenum format = GL_RGBA, bool flipVertical = false, bool flipHorizontal = false, bool useMipmaps = true, int filterType = GL_LINEAR);

