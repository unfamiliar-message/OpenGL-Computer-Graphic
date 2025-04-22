#include "TriangleMesh.h"
#include <fstream>
#include <cfloat>

extern GLuint vbo;
extern GLuint ibo;
// Desc: Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
	numVertices = 0;
	numTriangles = 0;
	objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	vboId = 0;
	iboId = 0;
}

// Desc: Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
	vertices.clear();
	vertexIndices.clear();
	glDeleteBuffers(1, &vboId);
	glDeleteBuffers(1, &iboId);
}

// Desc: Load the geometry data of the model from file and normalize it.
bool TriangleMesh::LoadFromFile(const std::string& filePath, const bool normalized)
{	
	// Add your code here.
	// ... 
	std::ifstream file("Cube.obj");
	if (!file.is_open()) {
		std::cerr << "無法打開文件。" << std::endl;
		return 1;
	}
	std::vector<glm::vec3> AllthePositions;
	std::vector<glm::vec2> AlltheMatltypes;
	std::vector<glm::vec3> AlltheNormals;

	std::string fline;
	float x, y, z;
	float MinX = std::numeric_limits<float>::max();
	float MaxX = std::numeric_limits<float>::min();
	float MinY = std::numeric_limits<float>::max();
	float MaxY = std::numeric_limits<float>::min();
	float MinZ = std::numeric_limits<float>::max();
	float MaxZ = std::numeric_limits<float>::min();
	while (std::getline(file, fline)) {
		if (fline.substr(0, 2) == "v ") {
			std::istringstream isa(fline.substr(2));
			if (isa >> x >> y >> z) {
				AllthePositions.push_back(glm::vec3(x, y, z));
				// 更新最大和最小值
				MinX = std::min(MinX, x);
				MaxX = std::max(MaxX, x);
				MinY = std::min(MinY, y);
				MaxY = std::max(MaxY, y);
				MinZ = std::min(MinZ, z);
				MaxZ = std::max(MaxZ, z);
			}
		}
		else if (fline.substr(0, 3) == "vt ") {
			std::istringstream isa(fline.substr(3));
			if (isa >> x >> y >> z) {
				AlltheMatltypes.push_back(glm::vec3(x, y, z));
			}
		}
		else if (fline.substr(0, 3) == "vn ") {
			std::istringstream isa(fline.substr(3));
			if (isa >> x >> y >> z) {
				AlltheNormals.push_back(glm::vec3(x, y, z));
			}
		}
	}
	file.seekg(0, std::ios::beg);
	std::vector<std::string> subStrings;
	int count;
	while (std::getline(file, fline)) {
		// Check whether the string starts with 'f'
		if (fline.find("f ") == 0) {
			fline = fline.substr(2); // delete former 'f '

			// Use string stream to split substrings
			std::istringstream iss(fline);
			std::string token;

			// Split string by space characters and store in vector
			while (iss >> token) {
				subStrings.push_back(token);
			}

			// Output the number of substrings separated by space characters
			count = subStrings.size();

			// Output a specific substring Output the substring stored in a vector
			std::cout << std::endl;

			// Analyze a specific substring
			bool newface=true;
			unsigned int Currentindex;
			unsigned int IndexbufferOnes=0;
			unsigned int FirstOnes,FormerOnes;
			glm::vec3 CurrentPosition, CurrentNormal;
			glm::vec2 Currentexcoord;
			std::istringstream CurrentString;
			int pointcount=0;
			while (count >= 3) {
				if (newface == true) {
					CurrentString.str(subStrings[0]);
					std::getline(CurrentString, token, '/'); //R1
					Currentindex = std::stoi(token);
					CurrentPosition = AllthePositions[Currentindex - 1];
					std::getline(CurrentString, token, '/'); //R2
					Currentindex = std::stoi(token);
					Currentexcoord = AlltheMatltypes[Currentindex - 1];
					std::getline(CurrentString, token);		 //R3
					Currentindex = std::stoi(token);
					CurrentNormal = AlltheNormals[Currentindex - 1];
					CurrentString.clear();
					//Put it into Vertexbuffer
					vertices.push_back(VertexPTN(CurrentPosition, CurrentNormal, Currentexcoord));
					// -----------
					//Put it into Indexbuffer
					vertexIndices.push_back(IndexbufferOnes);
					FirstOnes = IndexbufferOnes;
					IndexbufferOnes++;
					//-------
					for (int i = 1;i <= 2;i++) {
						CurrentString.str( subStrings[count - i] );
						std::getline(CurrentString, token, '/'); //R1
						Currentindex = std::stoi(token);
						CurrentPosition = AllthePositions[Currentindex - 1];
						std::getline(CurrentString, token, '/'); //R2
						Currentindex = std::stoi(token);
						Currentexcoord = AlltheMatltypes[Currentindex - 1];
						std::getline(CurrentString, token);		 //R3
						Currentindex = std::stoi(token);
						CurrentNormal = AlltheNormals[Currentindex - 1];
						CurrentString.clear();
						//Put it into vertices
						vertices.push_back(VertexPTN(CurrentPosition, CurrentNormal, Currentexcoord));
						// -----------
						//Put it into Indexbuffer
						vertexIndices.push_back(IndexbufferOnes);
						FormerOnes = IndexbufferOnes;
						IndexbufferOnes++;
						//-------
					}
					newface=false;
				}
				else {
					//Call and put  subStrings[0] into Indexbuffer 
					vertexIndices.push_back(FirstOnes);
					//Call and put  subStrings[count - 1] into Indexbuffer
					vertexIndices.push_back(FormerOnes);
					//----------------------------------------
					CurrentString.str( subStrings[count - 2] );
					std::getline(CurrentString, token, '/'); //R1
					Currentindex = std::stoi(token);
					CurrentPosition = AllthePositions[Currentindex - 1];
					std::getline(CurrentString, token, '/'); //R2
					Currentindex = std::stoi(token);
					Currentexcoord = AlltheMatltypes[Currentindex - 1];
					std::getline(CurrentString, token);		 //R3
					Currentindex = std::stoi(token);
					CurrentNormal = AlltheNormals[Currentindex - 1];
					CurrentString.clear();
					//Put it into vertices
					vertices.push_back(VertexPTN(CurrentPosition, CurrentNormal, Currentexcoord));
					//Put it into Indexbuffer
					vertexIndices.push_back(IndexbufferOnes);
					FormerOnes = IndexbufferOnes;
					IndexbufferOnes++;
					//-------
				};
				count--;
			}
		}	
	}
	file.close();
	//ConstructVertices

	numVertices = vertices.size();
	numTriangles = (vertexIndices.size())/3;
	PrintMeshInfo();
	return true;
}

// Desc: Create vertex buffer and index buffer.
void TriangleMesh::CreateBuffers()
{
	// Add your code here.
	// 三個以上請切割
	// Create vertex buffer //72 0526 needed vertex position
	//vertices //38 1600
	// 假設 N 是 vertexPosition 陣列的大小
	int N = vertices.size();
	glm::vec3* vertexPosition = new glm::vec3[N];

	for (int i = 0; i < N; ++i) {
		vertexPosition[i] = vertices[i].position;
	}
	// 現在 vertexPosition 陣列包含了 vertices 中每個元素的 position
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPosition),vertexPosition, GL_STATIC_DRAW);
	// Create vertex and index buffer.
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertexIndices) , &(*vertexIndices.begin()), GL_STATIC_DRAW);
	//-----------------------------------------------
}

// Desc: Apply transformation to all vertices (DON'T NEED TO TOUCH)
void TriangleMesh::ApplyTransformCPU(const glm::mat4x4& mvpMatrix)
{
	for (int i = 0 ; i < numVertices; ++i) {
        glm::vec4 p = mvpMatrix * glm::vec4(vertices[i].position, 1.0f);
        if (p.w != 0.0f) {
            float inv = 1.0f / p.w; 
            vertices[i].position.x = p.x * inv;
            vertices[i].position.y = p.y * inv;
            vertices[i].position.z = p.z * inv;
        }
    }
}

// Desc: Print mesh information.
void TriangleMesh::PrintMeshInfo() const
{
	std::cout << "[*] Mesh Information: " << std::endl;
	std::cout << "# Vertices: " << numVertices << std::endl;
	std::cout << "# Triangles: " << numTriangles << std::endl;
	std::cout << "Center: (" << objCenter.x << " , " << objCenter.y << " , " << objCenter.z << ")" << std::endl;
}
