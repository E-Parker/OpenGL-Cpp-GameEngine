#pragma once

#include <glad/glad.h>

struct Material;
struct Matrix;

typedef struct Mesh {
    /* This is the core structure of a mesh, it does not have any ability to manage itself at all. */

    size_t indices = 0;

    // Define GPU buffer objects:
    GLuint VertexAttributeObject = GL_NONE;       // Vertices with attributes that might be in different locations in the VBO. bind this to point to this mesh.
    GLuint VertexBufferObject = GL_NONE;          // raw vertex buffer.
    GLuint NormalBufferObject = GL_NONE;          // raw Normal buffer.
    GLuint TextureCoordBufferObject = GL_NONE;    // raw UV buffer.
    GLuint ElementBufferObject = GL_NONE;         // index of each vertex ructing faces. allows for all this to be done in one draw pass.

} Mesh;

void DrawRenderable( Mesh* mesh,  Material* material, const Matrix* transform);
void FreeMesh(Mesh* mesh);
void FreeSubMesh(Mesh* mesh);
void UploadMesh(Mesh* mesh,   uint32_t* indeciesArray,   Vector3* vertexBufferArray,   Vector3* normalBufferArray,  Vector2* tCoordArray,   size_t indecies,  size_t vertecies);
void UploadSubMesh(Mesh* mesh, Mesh* source,  uint32_t* indeciesArray,  uint32_t indecies);
