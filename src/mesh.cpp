#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>

#include "Camera.h"
#include "Mesh.h"
#include "Material.h"
#include "par_shapes.h"

const uint16_t LINE_BUFFER_SIZE = 512;
const uint16_t MAX_ITERATIONS = 0xffff;


StaticMesh::StaticMesh(uint16_t materialCount) : MaterialCount(materialCount) {
    meshRenders = new Mesh[materialCount];
    materials = new Material*[materialCount];
    Transform = MatrixIdentity();
}

StaticMesh::StaticMesh(uint16_t materialCount, Matrix transform) : MaterialCount(materialCount), Transform(transform) {
    meshRenders = new Mesh[materialCount];
    materials = new Material*[materialCount];
}

StaticMesh::~StaticMesh() {
    // not even going to bother with managing duplicate materials, this sucks enough as it is.
    // There is currently a memory leak caused by not deleting the materials.
    
    // Free the first mesh the normal way, since it contains the original reference to the vbo, tbo, and nbo.
    InternalFreeMesh(&meshRenders[0]);

    // Delete all other meshes with the subMesh method to avoid freeing the same location twice.
    for (int i = 1; i < MaterialCount; i++) {
        InternalFreeSubMesh(&meshRenders[i]);
    
}
    
    delete[] materials;
    materials = nullptr;

    delete[] meshRenders;
    meshRenders = nullptr;
}

void StaticMesh::SetMaterial(Material* material, uint16_t index) {

    if(this == nullptr || index >= MaterialCount) {
        return;
    }

    materials[index] = material;
}

void InternalFreeSubMesh(Mesh* mesh) {
    /* Use this to free a mesh that was created by copying from another. */
    
    if (mesh->ElementBufferObject != GL_NONE) {
        glDeleteBuffers(1, &(mesh->ElementBufferObject));
        mesh->ElementBufferObject = GL_NONE;
    }

    if (mesh->VertexAttributeObject != GL_NONE) {
        glDeleteBuffers(1, &(mesh->VertexAttributeObject));
        mesh->VertexAttributeObject = GL_NONE;
    }

}

void InternalFreeMesh(Mesh* mesh) {

    if (mesh->ElementBufferObject != GL_NONE) {
        glDeleteBuffers(1, &(mesh->ElementBufferObject));
        mesh->ElementBufferObject = GL_NONE;
    }

    if (mesh->TextureCoordBufferObject != GL_NONE) {
        glDeleteBuffers(1, &(mesh->TextureCoordBufferObject));
        mesh->TextureCoordBufferObject = GL_NONE;
    }

    if (mesh->NormalBufferObject != GL_NONE) {
        glDeleteBuffers(1, &(mesh->NormalBufferObject));
        mesh->NormalBufferObject = GL_NONE;
    }

    if (mesh->VertexBufferObject != GL_NONE) {
        glDeleteBuffers(1, &(mesh->VertexBufferObject));
        mesh->VertexBufferObject = GL_NONE;
    }

    if (mesh->VertexAttributeObject != GL_NONE) {
        glDeleteBuffers(1, &(mesh->VertexAttributeObject));
        mesh->VertexAttributeObject = GL_NONE;
    }
}

void InternalUploadMesh(Mesh* mesh, Mesh* source, const uint16_t* indeciesArray, const uint16_t indecies){
    /* variant of UploadMesh for meshes that share vertices but have a different element buffer. */
    
    size_t indexBytes = indecies * sizeof(uint16_t);
    mesh->indexBytes = indexBytes;

    if (mesh->VertexAttributeObject == GL_NONE) { 
        glGenVertexArrays(1, &(mesh->VertexAttributeObject));
    }
    glBindVertexArray(mesh->VertexAttributeObject);

    mesh->VertexBufferObject = source->VertexBufferObject;
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VertexBufferObject);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), nullptr);
    glEnableVertexAttribArray(0);

    mesh->NormalBufferObject = source->NormalBufferObject;
    glBindBuffer(GL_ARRAY_BUFFER, mesh->NormalBufferObject);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), nullptr);
    glEnableVertexAttribArray(1);
       
    mesh->TextureCoordBufferObject = source->TextureCoordBufferObject;
    glBindBuffer(GL_ARRAY_BUFFER, mesh->TextureCoordBufferObject);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2), nullptr);
    glEnableVertexAttribArray(2);

    if (mesh->ElementBufferObject == GL_NONE) { glGenBuffers(1, &(mesh->ElementBufferObject)); }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ElementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBytes, indeciesArray, GL_STATIC_DRAW);

    glBindVertexArray(GL_NONE);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);

}

void InternalUploadMesh(Mesh* mesh, const  uint16_t* indeciesArray, const  Vector3* vertexBufferArray, const  Vector3* normalBufferArray, const Vector2* tCoordArray, const  size_t indecies, const  size_t vertecies) {
    /* Uploading mesh to GPU. points and normalBuffer must exist for the upload to work. 
    tCoord data and face data is optional. */

    size_t vertexBytes = vertecies * sizeof(Vector3);
    size_t tCoordBytes = vertecies * sizeof(Vector2);
    size_t indexBytes = indecies * sizeof(uint16_t);
    size_t normalBytes = vertexBytes;

    mesh->indexBytes = indexBytes;

    // Create a Vertex Attribute Object. This is kind of like a container for the buffer objects.              
    if (mesh->VertexAttributeObject == GL_NONE) { glGenVertexArrays(1, &(mesh->VertexAttributeObject)); }
    glBindVertexArray(mesh->VertexAttributeObject);
    
    // This buffer is bound to the 0th attribute, it stores the points of the mesh.
    if (mesh->VertexBufferObject == GL_NONE) { glGenBuffers(1, &(mesh->VertexBufferObject)); }
    glBindBuffer(GL_ARRAY_BUFFER, (mesh->VertexBufferObject));
    glBufferData(GL_ARRAY_BUFFER, vertexBytes, vertexBufferArray, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), nullptr);
    glEnableVertexAttribArray(0);

    // This buffer is bound to the 1st Attribute, it stores the normal vectors for each point.
    if (mesh->NormalBufferObject == GL_NONE) { glGenBuffers(1, &(mesh->NormalBufferObject)); }
    glBindBuffer(GL_ARRAY_BUFFER, mesh->NormalBufferObject);
    glBufferData(GL_ARRAY_BUFFER, normalBytes, normalBufferArray, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), nullptr);
    glEnableVertexAttribArray(1);

    // First we check if the mesh has texture coordinates, then we add a buffer and assign it.
    if (tCoordArray != nullptr) {
        if (mesh->TextureCoordBufferObject == GL_NONE) { glGenBuffers(1, &(mesh->TextureCoordBufferObject)); }
        glBindBuffer(GL_ARRAY_BUFFER, mesh->TextureCoordBufferObject);
        glBufferData(GL_ARRAY_BUFFER, tCoordBytes, tCoordArray, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2), nullptr);
        glEnableVertexAttribArray(2);
    }
    else {
        std::cout << "Warning: mesh does not have texture coordinates." << std::endl;
    }

    // First check if there are face indicies, then make an element array for them.
    if (indeciesArray != nullptr) {
        if(mesh->ElementBufferObject == GL_NONE) { glGenBuffers(1, &(mesh->ElementBufferObject)); }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ElementBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBytes, indeciesArray, GL_STATIC_DRAW);
    }
    else {
        std::cout << "Warning: faces not provided, using naive method." << std::endl;
    }

    glBindVertexArray(GL_NONE);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);

}

void StaticMesh::Draw(Camera* camera) const {
    /* function to draw a mesh on screen. */

    if (this == nullptr) {
        return;
    }

    Matrix mvp = Transform * camera->ViewMatrix;

    // run a draw call for each material.
    for (uint16_t i = 0; i < MaterialCount; i++) {
         
        // ensure the current material is valid.
        if (materials[i] == nullptr) {
            continue;
        }
            
        // Bind the material's shader program and textures.
        materials[i]->BindMaterial();

        // Get the uniform from the shader.
        GLint uniform = glGetUniformLocation(materials[i]->Program, "u_mvp");

        // Bind the VAO and draw the elements.
        glBindVertexArray(meshRenders[i].VertexAttributeObject);
        glUniformMatrix4fv(uniform, 1, GL_FALSE, ToFloat16(mvp).v);
        glDrawElements(GL_TRIANGLES, meshRenders[i].indexBytes, GL_UNSIGNED_SHORT, 0);
        
    }
    // unbind the VAO.
    glBindVertexArray(GL_NONE);
}

Vector2 Vector2FromString(const std::string data) {

	std::stringstream dataStream(data);
	std::string segment;
	std::vector<std::string> segmentList;

	while(std::getline(dataStream, segment, ' ')) {
        if (!segment.empty()) {
            segmentList.push_back(segment);
        }
	}

    Vector2 vector{0.0f, 0.0f};

	vector.x = std::stof(segmentList[0]);
	vector.y = std::stof(segmentList[1]);

	return vector;
	
}

Vector3 Vector3FromString(const std::string data) {

	std::stringstream dataStream(data);
	std::string segment;
	std::vector<std::string> segmentList;

	while(std::getline(dataStream, segment, ' ')) {
        if (!segment.empty()) {
   		    segmentList.push_back(segment);
        }
	}

    Vector3 vector{0.0f, 0.0f, 0.0f};

	vector.x = std::stof(segmentList[0]);
	vector.y = std::stof(segmentList[1]);
	vector.z = std::stof(segmentList[2]);

	return vector;
}

void parseFace(std::vector<uint16_t>* vi, std::vector<uint16_t>* ti, std::vector<uint16_t>* ni, std::vector<std::string> segmentList) {
    
    // For each of the three points in the face,
    for (int i = 0; i < segmentList.size(); i++) {
        std::stringstream subDataStream(segmentList[i]);
        std::string subSegment;
        std::vector<std::string> subSegmentList;

        while (std::getline(subDataStream, subSegment, '/')) {
            subSegmentList.push_back(subSegment);
        }

        // for each subset, get the int stored there.
        // This kind of sucks but it means that if there's a missing index nothing is added for that list.
        // TODO: add support for models that lack UV or Normal data.
        for (int k = 0; k < subSegmentList.size(); k++) {
            switch (k) {
            case 0:	vi->push_back(std::stoi(subSegmentList[0]) - 1); break;
            case 1:	ti->push_back(std::stoi(subSegmentList[1]) - 1); break;
            case 2:	ni->push_back(std::stoi(subSegmentList[2]) - 1); break;
            default: break;
            }
        }
    }
}

int parseFaceIndicies(std::vector<uint16_t>* vi, std::vector<uint16_t>* ti, std::vector<uint16_t>* ni, const std::string data) {
	/* This function parses an incoming wavefront file face data and added the indicies to the corresponding lists. 
	Faces are stored as f n/n/n n/n/n n/n/n where the subsets are the indicies of the vertex, texture coordinate, and normal. */

	std::stringstream dataStream(data);
	std::string segment;
	std::vector<std::string> segmentList;

	while(std::getline(dataStream, segment, ' ')) {
   		segmentList.push_back(segment);
	}

    if (segmentList.size() == 3) {
        parseFace(vi, ti, ni, segmentList);
        return 3;
    }

    if (segmentList.size() == 4) {
        std::vector<std::string> firstTriangle { segmentList[0], segmentList[1], segmentList[2] };
        std::vector<std::string> secondTriangle{ segmentList[2], segmentList[3], segmentList[0] };

        parseFace(vi, ti, ni, firstTriangle);
        parseFace(vi, ti, ni, secondTriangle);
    }
    return 6;
}

StaticMesh* CreateStaticMeshFromWavefront(const char* path) {
    /* Parse an obj file and load a mesh from it. */ 
    
    // Interpret the file as a giant string
    std::stringstream stream;

    try {
        std::ifstream file;
	    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);    // Set exceptions for try catch.
        file.open(path);                                                    // Attempt to open the file.
        stream << file.rdbuf();
        file.close();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "Wavefront (" << path << ") not found: " << e.what() << std::endl;
        return nullptr;
    }

    // Verify that the file extension is obj.
    const char* ext = strrchr(path, '.');
    assert(strcmp(ext, ".obj") == 0 || strcmp(ext, ".OBJ") == 0);

    char lineBuffer[LINE_BUFFER_SIZE];
    char identifyer[2] = { '\0', '\0' };
    uint16_t lineCount = 0;
    uint16_t materialCount = 0;

    std::string ObjectName = "None";
    std::vector<uint16_t> surfaceSplitIndecies;
	std::vector<uint16_t> vi; 
	std::vector<uint16_t> ti; 
	std::vector<uint16_t> ni; 
	
	std::vector<Vector3> vertexList;
	std::vector<Vector3> normalList;
	std::vector<Vector2> tCoordList;

    stream.seekp(0);
    uint16_t iteration = 0;
    uint16_t indiciesParced = 0;
	
    while (!stream.eof() || ++iteration < MAX_ITERATIONS) {
    
    	// Copy the line into the buffer:
    	stream.getline(lineBuffer, LINE_BUFFER_SIZE);       // Get the current line from the file.
    	memcpy(identifyer, lineBuffer, 2 * sizeof(char));   // Copy the first two characters from the line buffer.
		
		// Quick hack to get it as a string.
		std::string lineBufferString = std::string(lineBuffer);

    	switch (identifyer[0]) {
    	case '#':   // comment line, skip this one.
            break;

        case 'o':   // object name.
            ObjectName = lineBufferString.substr(2);
            break;

        case 's':   // use smooth shading or not.
                  
            break;

        case 'u':   // material.
            
            if (identifyer[1] != 's') {
                break;
            }

            materialCount++;
            surfaceSplitIndecies.push_back(indiciesParced);
            indiciesParced = 0;
            break;

    	case 'f':
    	    //handle face stuff here.
            indiciesParced += parseFaceIndicies(&vi, &ti, &ni, lineBufferString.substr(2));
            break;

        // handle all vertex cases.
    	case 'v':
    	    switch (identifyer[1]) {
    	    case ' ':   // vertex
                vertexList.push_back(Vector3FromString(lineBufferString.substr(2)));
				break;
    	    case 't':   // tCoord
                tCoordList.push_back(Vector2FromString(lineBufferString.substr(3)));
				break;
    	    case 'n':   // normal
                normalList.push_back(Vector3FromString(lineBufferString.substr(3)));
                break;
    	    }
        default:
            break;
        }
	}
    
    if (surfaceSplitIndecies.empty()) {
        surfaceSplitIndecies.push_back(indiciesParced);
    }
    else {
        surfaceSplitIndecies[0] = indiciesParced;
    }

    // Create a new mesh from the index data which follows the standards of OpenGL.

    // Assert that all index list are the same size.
    assert(vi.size() == ni.size() && ni.size() == ti.size());

    std::vector<Vector3> normalArray(vi.size(), Vector3{ 0.0f, 1.0f, 0.0f });
    std::vector<Vector2> tCoordArray(vi.size(), Vector2{ 0.0f, 0.0f });

    for (uint16_t i = 0; i < vi.size(); i++) {
        normalArray[vi[i]] = normalList[ni[i]];
        tCoordArray[vi[i]] = tCoordList[ti[i]];
    }

    StaticMesh* newMesh = new StaticMesh((materialCount == 0) ? 1 : materialCount, MatrixIdentity());
    SetAlias(newMesh, ObjectName.c_str());

    // upload the first mesh containing the actual vertex, normal and tChoord buffers. 
    InternalUploadMesh(&(newMesh->meshRenders[0]), &vi[0], &vertexList[0], &normalArray[0], &tCoordArray[0], surfaceSplitIndecies[0], vertexList.size());
    
    //starting at the first material split, upload a sub-mesh referencing the buffers from the first mesh.
    uint16_t currentMaterialElementIndex = 0;

    for (uint16_t i = 1; i < materialCount; i++) {
        // Copy the vbo, tbo, and nbo from the first mesh which holds all the data.
        currentMaterialElementIndex += surfaceSplitIndecies[i];
        InternalUploadMesh(&newMesh->meshRenders[i], &newMesh->meshRenders[0], &vi[currentMaterialElementIndex], surfaceSplitIndecies[i]);
    }
	return newMesh;
}


StaticMesh* CreateStaticMeshFromRawData(const uint16_t* indeciesArray, const  Vector3* vertexBufferArray, const  Vector3* normalBufferArray, const  Vector2* tCoordArray, const  size_t indecies, const  size_t vertecies) {
    StaticMesh* newMesh = new StaticMesh(1, MatrixIdentity());
    InternalUploadMesh(&(newMesh->meshRenders[0]), indeciesArray, vertexBufferArray, normalBufferArray, tCoordArray, indecies, vertecies);
    return newMesh;
}


StaticMesh* CreateStaticMeshPrimativeCone(int slices, int stacks) {
    par_shapes_mesh* parMesh = par_shapes_create_cone(slices, stacks);
    StaticMesh* newMesh = new StaticMesh(1, MatrixIdentity());
    InternalUploadMesh(&(newMesh->meshRenders[0]), parMesh->triangles, (Vector3*)parMesh->points, (Vector3*)parMesh->normals, (Vector2*)parMesh->tcoords, parMesh->ntriangles * 3, parMesh->npoints);
    par_shapes_free_mesh(parMesh);
    return newMesh;
}


StaticMesh* CreateStaticMeshPrimativeCylinder(int slices, int stacks) {
    par_shapes_mesh* parMesh = par_shapes_create_cylinder(slices, stacks);
    StaticMesh* newMesh = new StaticMesh(1, MatrixIdentity());
    InternalUploadMesh(&(newMesh->meshRenders[0]), parMesh->triangles, (Vector3*)parMesh->points, (Vector3*)parMesh->normals, (Vector2*)parMesh->tcoords, parMesh->ntriangles * 3, parMesh->npoints);
    par_shapes_free_mesh(parMesh);
    return newMesh;
}


StaticMesh* CreateStaticMeshPrimativeTorus(int slices, int stacks, float radius) {
    par_shapes_mesh* parMesh = par_shapes_create_torus(slices, stacks, radius);
    StaticMesh* newMesh = new StaticMesh(1, MatrixIdentity());
    InternalUploadMesh(&(newMesh->meshRenders[0]), parMesh->triangles, (Vector3*)parMesh->points, (Vector3*)parMesh->normals, (Vector2*)parMesh->tcoords, parMesh->ntriangles * 3, parMesh->npoints);
    par_shapes_free_mesh(parMesh);
    return newMesh;
}


StaticMesh* CreateStaticMeshPrimativePlane(int slices, int stacks) {
    par_shapes_mesh* parMesh = par_shapes_create_plane(slices, stacks);
    StaticMesh* newMesh = new StaticMesh(1, MatrixIdentity());
    InternalUploadMesh(&(newMesh->meshRenders[0]), parMesh->triangles, (Vector3*)parMesh->points, (Vector3*)parMesh->normals, (Vector2*)parMesh->tcoords, parMesh->ntriangles * 3, parMesh->npoints);
    par_shapes_free_mesh(parMesh);
    return newMesh;
}


StaticMesh* CreateStaticMeshPrimativeSphere(int subdivisions) {
    par_shapes_mesh* parMesh = par_shapes_create_subdivided_sphere(subdivisions);
    StaticMesh* newMesh = new StaticMesh(1, MatrixIdentity());
    InternalUploadMesh(&(newMesh->meshRenders[0]), parMesh->triangles, (Vector3*)parMesh->points, (Vector3*)parMesh->normals, (Vector2*)parMesh->tcoords, parMesh->ntriangles * 3, parMesh->npoints);
    par_shapes_free_mesh(parMesh);
    return newMesh;
}


StaticMesh* CreateStaticMeshFromGraphicsLibraryTransmissionFormat(const char* Path) {
    return nullptr; 
}


StaticMesh* CreateStaticMeshFromGraphicsLibraryBinaryTransmissionFormat(const char* Path) {
    return nullptr;
}


