#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "headers.h"
#include "material.h"
#include <fstream>
#include <sstream>
// VertexPTN Declarations.
struct VertexPTN
{
	VertexPTN() {
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		normal = glm::vec3(0.0f, 1.0f, 0.0f);
		texcoord = glm::vec2(0.0f, 0.0f);
	}
	VertexPTN(glm::vec3 p, glm::vec3 n, glm::vec2 uv) {
		position = p;
		normal = n;
		texcoord = uv;
	}
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
};

// SubMesh Declarations.
struct SubMesh
{
	SubMesh() {
		material = nullptr;
		iboId = 0;
	}
	PhongMaterial* material;
	GLuint iboId;
	std::vector<unsigned int> vertexIndices;
};


// TriangleMesh Declarations.
class TriangleMesh
{
public:
	// TriangleMesh Public Methods.
	TriangleMesh();
	~TriangleMesh();

	// Load the model from an *.OBJ file.
	bool LoadFromFile(const std::string& filePath, const bool normalized = true);
	//Load the Model of mtl file
	bool LoadFrom_mtlfile(const std::string& filePath, const bool normalized = true);
	// Show model information.
	void ShowInfo();


	//create buffer

	void CreateBuffers();
	void Draw(PhongShadingDemoShaderProg*);
	// -------------------------------------------------------
	void getmax();
	void getmin();
	float scale();
	void adjust_to_0();
	void Reconsruct_vertices();;//Deal with the problem of different normals at the same point and process the data in vertices.
	void updatedata();

	// -------------------------------------------------------

	int GetNumVertices() const { return numVertices; }
	int GetNumTriangles() const { return numTriangles; }
	int GetNumSubMeshes() const { return (int)subMeshes.size(); }


private:
	// -------------------------------------------------------
	float max_x, max_y, max_z;
	float min_x, min_y, min_z;
	float maxrange_x, maxrange_y, maxrange_z;
	// -------------------------------------------------------

	// TriangleMesh Private Data.
	GLuint vboId;

	std::vector<VertexPTN> vertices;
	std::vector<glm::vec3> vertices_normal;//normal vertex
	std::vector<glm::vec2> vertices_texture;//texture vertex
	std::vector<unsigned int> vertices_normal_index;//normal index buffer
	std::vector<unsigned int> vertices_texture_index;//texture index buffer
	// For supporting multiple materials per object, move to SubMesh.
	//GLuint iboId;
	std::vector<unsigned int> vertexIndices;
	std::vector<SubMesh> subMeshes;
	std::map<std::string, PhongMaterial> phongmaterialpair;//material corresponding to the name


	int numVertices;
	int numTriangles;
	glm::vec3 objCenter;
	glm::vec3 objExtent;
};


#endif
