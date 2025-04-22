#include "trianglemesh.h"
//Test
// Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
    // ---------------------------------------------------------------------------
    numVertices = 0;
    numTriangles = 0;
    objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
    objExtent = glm::vec3(0.0f, 0.0f, 0.0f);
    vboId = 0;
    max_x = max_y = max_z = 0;
    min_x = min_y = min_z = 0;
    maxrange_x = maxrange_y = maxrange_z = 0;
    // ---------------------------------------------------------------------------
}

// Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
    // -------------------------------------------------------
    vertices.clear();
    subMeshes.clear();
    // -------------------------------------------------------
}

// Load the geometry and material data from an OBJ file.
bool TriangleMesh::LoadFromFile(const std::string& filePath, const bool normalized)
{
    // Parse the OBJ file.
    // ---------------------------------------------------------------------------
    std::string res, line;
    std::ifstream fin(filePath);
    if (!fin.is_open())
    {
        std::cout << "file: " << filePath << " fails to open." << std::endl;
        exit(-1);
    }
    GLuint submesh_index = 0;
    // ---------------------------------------------------------------------------
    // Normalize the geometry data.
    if (normalized) {
        SubMesh* submesh = new SubMesh;

        while (std::getline(fin, line))
        {
            VertexPTN ptn;
            std::istringstream sin(line);   // Parse and read one line of data as a string stream
            std::string type, mtlfilename;
            GLfloat v0, vt0, vn0, v1, vt1, vn1, v2, vt2, vn2, x, y, z;
            char slash;

            // Read the .obj file
            sin >> type;
            if (type == "mtllib") {
                sin >> mtlfilename;
                LoadFrom_mtlfile(mtlfilename, true);
            }
            if (type == "v") {  //vertex
                sin >> x >> y >> z;
                ptn.position = glm::vec3(x, y, z);
                vertices.push_back(ptn);
                numVertices++;
            }

            if (type == "vn") {  //normal

                sin >> x >> y >> z;
                vertices_normal.push_back(glm::vec3(x, y, z));
            }
            if (type == "usemtl") {
                std::string mtlname;

                if (submesh_index) {
                    subMeshes.push_back(*submesh);
                }
                delete submesh;
                submesh = new SubMesh;
                sin >> mtlname;
                submesh->material = &phongmaterialpair[mtlname];
                submesh_index++;

            }
            if (type == "f") {
                sin >> v0 >> slash >> vt0 >> slash >> vn0;
                sin >> v1 >> slash >> vt1 >> slash >> vn1;
                sin >> v2;
                GLfloat temp_v = abs(v1);
                GLfloat temp_vt = abs(vt1);
                GLfloat temp_vn = abs(vn1);
                do {
                    sin >> slash >> vt2 >> slash >> vn2;

                    submesh->vertexIndices.push_back(abs(v0) - 1);
                    submesh->vertexIndices.push_back(temp_v - 1);
                    submesh->vertexIndices.push_back(abs(v2) - 1);

                    vertices_normal_index.push_back(abs(vn0) - 1);
                    vertices_normal_index.push_back(temp_vn - 1);
                    vertices_normal_index.push_back(abs(vn2) - 1);

                    temp_v = abs(v2);
                    temp_vt = abs(vt2);
                    temp_vn = abs(vn2);

                    numTriangles++;
                } while (sin >> v2);
            }
        }
        subMeshes.push_back(*submesh);

        // -----------------------------------------------------------------------
    }
    Reconsruct_vertices();


    //normalize
    getmax();   getmin();
    adjust_to_0();//centerlize to (0,0,0)
    updatedata();
    CreateBuffers();
    return true;
}

// Show model information.
void TriangleMesh::ShowInfo()
{
    std::cout << "# Vertices: " << numVertices << std::endl;
    std::cout << "# Triangles: " << numTriangles << std::endl;
    std::cout << "Total " << subMeshes.size() << " subMeshes loaded" << std::endl;
    for (unsigned int i = 0; i < subMeshes.size(); ++i) {
        const SubMesh& g = subMeshes[i];
        std::cout << "SubMesh " << i << " with material: " << g.material->GetName() << std::endl;
        std::cout << "Num. triangles in the subMesh: " << g.vertexIndices.size() / 3 << std::endl;
    }
    std::cout << "Model Center: " << objCenter.x << ", " << objCenter.y << ", " << objCenter.z << std::endl;
    std::cout << "Model Extent: " << objExtent.x << " x " << objExtent.y << " x " << objExtent.z << std::endl;
}
void  TriangleMesh::getmax() {
    float _max_y, _max_x, _max_z;
    _max_y = _max_x = _max_z = FLT_MIN;
    for (int i = 0; i < vertices.size(); i++) {
        if (vertices[i].position.y > _max_y) {
            _max_y = vertices[i].position.y;
        }
        if (vertices[i].position.z > _max_z) {
            _max_z = vertices[i].position.z;
        }
        if (vertices[i].position.x > _max_x) {
            _max_x = vertices[i].position.x;
        }
    }
    max_x = _max_x;
    max_y = _max_y;
    max_z = _max_z;
}
void  TriangleMesh::getmin() {
    float _min_y, _min_x, _min_z;
    _min_y = _min_x = _min_z = FLT_MAX;
    for (int i = 0; i < vertices.size(); i++) {
        if (vertices[i].position.x < _min_x) {
            _min_x = vertices[i].position.x;
        }
        if (vertices[i].position.y < _min_y) {
            _min_y = vertices[i].position.y;
        }
        if (vertices[i].position.z < _min_z) {
            _min_z = vertices[i].position.z;
        }
    }
    min_x = _min_x;
    min_y = _min_y;
    min_z = _min_z;
}
float TriangleMesh::scale() {
    return std::max(std::max(max_x - min_x, max_y - min_y), max_z - min_z);
}
void TriangleMesh::adjust_to_0() {
    maxrange_x = (max_x + min_x) / 2;
    maxrange_y = (max_y + min_y) / 2;
    maxrange_z = (max_z + min_z) / 2;
    objExtent = glm::vec3((max_x - min_x), (max_y - min_y), (max_z - min_z));
    objCenter = glm::vec3(maxrange_x, maxrange_y, maxrange_z);
    for (int i = 0; i < vertices.size(); i++) {
        vertices[i].position.x -= maxrange_x;
        vertices[i].position.y -= maxrange_y;
        vertices[i].position.z -= maxrange_z;
    }

}
void TriangleMesh::updatedata() {
    objCenter -= glm::vec3(maxrange_x, maxrange_y, maxrange_z);
    objExtent /= glm::vec3(scale(), scale(), scale());
}
//Åª¨úmtlÀÉ®×
bool TriangleMesh::LoadFrom_mtlfile(const std::string& filePath, const bool normalized) {
    std::string res, line;
    std::ifstream fin(filePath);
    PhongMaterial temp;//PhongMaterial temporary
    if (!fin.is_open())
    {
        std::cout << "mtl file: " << filePath << "fail to open." << std::endl;
        exit(-1);
    }
    if (normalized) {
        while (std::getline(fin, line))
        {
            std::istringstream sin(line);
            std::string mtltype;
            std::string type;
            int num = 0;
            GLfloat x, y, z;
            sin >> type;
            //divide the model as submesh when "new"
            if (type == "newmtl") {
                phongmaterialpair[temp.GetName()] = temp;
                sin >> mtltype;
                temp.SetName(mtltype);

            }
            if (type == "Ns") {
                sin >> x;
                temp.SetNs(x);
            }
            if (type == "Ka") {
                sin >> x >> y >> z;
                temp.SetKa(glm::vec3(x, y, z));
            }
            if (type == "Ks") {
                sin >> x >> y >> z;
                temp.SetKs(glm::vec3(x, y, z));
            }
            if (type == "Kd") {
                sin >> x >> y >> z;
                temp.SetKd(glm::vec3(x, y, z));
            }


        }
        phongmaterialpair[temp.GetName()] = temp;
        return true;
    }
}
void TriangleMesh::CreateBuffers()
{
    //deal vbo
    glDeleteVertexArrays(1, &vboId);
    glGenBuffers(1, &vboId);
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPTN) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
    //deal ibo
    for (int i = 0; i < subMeshes.size(); i++) {
        glDeleteBuffers(1, &subMeshes[i].iboId);

        glGenBuffers(1, &subMeshes[i].iboId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMeshes[i].iboId);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * subMeshes[i].vertexIndices.size(), &subMeshes[i].vertexIndices[0], GL_STATIC_DRAW);
    }
}
void TriangleMesh::Draw(PhongShadingDemoShaderProg* phongShadingShader)
{
    for (int i = 0; i < subMeshes.size(); i++) {

        glUniform3fv(phongShadingShader->GetLocKa(), 1, glm::value_ptr(subMeshes[i].material->GetKa()));
        glUniform3fv(phongShadingShader->GetLocKd(), 1, glm::value_ptr(subMeshes[i].material->GetKd()));
        glUniform3fv(phongShadingShader->GetLocKs(), 1, glm::value_ptr(subMeshes[i].material->GetKs()));
        glUniform1f(phongShadingShader->GetLocNs(), subMeshes[i].material->GetNs());

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, vboId);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (const GLvoid*)12);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subMeshes[i].iboId);
        glDrawElements(GL_TRIANGLES, subMeshes[i].vertexIndices.size(), GL_UNSIGNED_INT, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
}
void TriangleMesh::Reconsruct_vertices() {

    int count = 0;
    for (int i = 0; i < subMeshes.size(); i++) {
        for (int j = 0; j < subMeshes[i].vertexIndices.size(); j++) {
            // let the normal enter in VertexPTN

            if (vertices[subMeshes[i].vertexIndices[j]].normal == glm::vec3(0.0f, 1.0f, 0.0f)) {
                vertices[subMeshes[i].vertexIndices[j]].normal = vertices_normal[vertices_normal_index[count + j]];

            }
            else {
                VertexPTN ptn;
                ptn.position = vertices[subMeshes[i].vertexIndices[j]].position;
                ptn.normal = vertices_normal[vertices_normal_index[count + j]];
                vertices.push_back(ptn);
                subMeshes[i].vertexIndices[j] = vertices.size() - 1;
            }

        }
        count += subMeshes[i].vertexIndices.size();
    }
}
