#pragma once
#include <vector>
#include "asset.h"

//Forward Definitions:
class Material;

typedef struct Mesh {
    /* This is the core structure of a mesh, it does not have any ability to manage itself at all. */

    GLsizei indexBytes = 0;

    // Define GPU buffer objects:
    GLuint VertexAttributeObject = GL_NONE;       // Vertices with attributes that might be in different locations in the VBO. bind this to point to this mesh.
    GLuint VertexBufferObject = GL_NONE;          // raw vertex buffer.
    GLuint NormalBufferObject = GL_NONE;          // raw Normal buffer.
    GLuint TextureCoordBufferObject = GL_NONE;    // raw UV buffer.
    GLuint ElementBufferObject = GL_NONE;         // index of each vertex constructing faces. allows for all this to be done in one draw pass.

} Mesh;


struct StaticMesh {
    /* Data structure to store mesh data. */

    ASSET_BODY(ObjectType::StaticMesh)
    Mesh* meshRenders;
    Material** materials;
    uint16_t MaterialCount;

    StaticMesh(uint16_t MaterialCount);
    StaticMesh(uint16_t MaterialCount, Matrix transform);
    ~StaticMesh();

    void SetMaterial(Material* material, uint16_t index);

    void Draw(Camera* camera) const;

};

StaticMesh* CreateStaticMeshFromRawData(const uint16_t* indeciesArray, const  Vector3* vertexBufferArray, const  Vector3* normalBufferArray, const  Vector2* tCoordArray, const  size_t indecies, const  size_t vertecies);
StaticMesh* CreateStaticMeshFromGraphicsLibraryTransmissionFormat(const char* Path);
StaticMesh* CreateStaticMeshFromGraphicsLibraryBinaryTransmissionFormat(const char* Path);
StaticMesh* CreateStaticMeshFromWavefront(const char* path);

// Par-Shapes wrapers:
StaticMesh* CreateStaticMeshPrimativeCone(int slices, int stacks);
StaticMesh* CreateStaticMeshPrimativeCylinder(int slices, int stacks);
StaticMesh* CreateStaticMeshPrimativeTorus(int slices, int stacks, float radius);
StaticMesh* CreateStaticMeshPrimativePlane(int slices, int stacks);
StaticMesh* CreateStaticMeshPrimativeSphere(int subdivisions);

void InternalUploadMesh(Mesh* mesh, Mesh* source, const uint16_t* indeciesArray, const uint16_t indecies);
void InternalUploadMesh(Mesh* mesh, const  uint16_t* indeciesArray, const  Vector3* vertexBufferArray, const  Vector3* normalBufferArray, const Vector2* tCoordArray, const  size_t indecies, const  size_t vertecies);
void InternalFreeSubMesh(Mesh* mesh);
void InternalFreeMesh(Mesh* mesh);

// String parsing:
static void parseFace(std::vector<uint16_t>* vi, std::vector<uint16_t>* ti, std::vector<uint16_t>* ni, std::vector<std::string> segmentList);
static int parseFaceIndicies(std::vector<uint16_t>* vi, std::vector<uint16_t>* ti, std::vector<uint16_t>* ni, const std::string data);
static Vector2 Vector2FromString(const std::string data);
static Vector3 Vector3FromString(const std::string data);


