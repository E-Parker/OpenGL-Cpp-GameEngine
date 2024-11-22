#pragma once

//Forward Declarations
template<typename T> class HashTable;
struct Texture;


typedef struct Uniform {
    /* struct to store openGL uniform information. This is still very WIP. */

    char* alias;    //name of the uniform.
    GLint location; // GPU location of the uniform.
    void* data;     // Arbetrary data that differs depending on the uniform type.
    

} Uniform;

typedef struct Material {

public:

    uint16_t TexturesUsed = 0;
    Texture** Textures;

    HashTable<GLint>* Uniforms;
    
    GLuint Program = GL_NONE;
    GLenum CullFunction = GL_BACK;
    GLenum DepthFunction = GL_LESS;

    Material(const char* vertexProgramPath, const char* fragmentProgramPath, const uint16_t numberOfTextures, const GLenum cullFuncton, const GLenum depthFunction);
    ~Material();

} Material;

void SetTextureFromPointer(const Material* material, Texture* texture, uint16_t index);
void SetTextureFromAlias(const Material* material, const char* alias, uint16_t index);
void BindMaterial(const Material* material);
