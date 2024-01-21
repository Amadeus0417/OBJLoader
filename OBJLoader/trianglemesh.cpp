#include "trianglemesh.h"

const std::vector<std::string> split(const std::string& str, const char& delimiter) {
	std::vector<std::string> result;
	std::stringstream ss(str);
	std::string token;

	while (std::getline(ss, token, delimiter)) {
		result.push_back(token);
	}
	return result;
}

// Constructor of a triangle mesh.
TriangleMesh::TriangleMesh()
{
	vboId = 0;
	numVertices = 0;
	numTriangles = 0;
	objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	objExtent = glm::vec3(0.0f, 0.0f, 0.0f);
}

// Destructor of a triangle mesh.
TriangleMesh::~TriangleMesh()
{
	vertices.clear();
	subMeshes.clear();
	glDeleteBuffers(1, &vboId);
}

// Load the geometry and material data from an OBJ file.
bool TriangleMesh::LoadFromFile(const std::string& filePath, const bool normalized)
{
	// Parse the OBJ file.
	// ---------------------------------------------------------------------------
	// Add your implementation here (HW1 + read *.MTL).
	// ---------------------------------------------------------------------------
	std::vector<glm::vec3> positions, normals;
	std::vector<glm::vec2> texcoords;
	std::vector<std::vector<glm::vec3>> SubMeshFaceData;
	std::vector<glm::vec3> SingleSubMeshFaceData = {};
	std::vector<std::string> MaterialName;
	std::string s;
	std::ifstream file(filePath, std::ios::in);
	std::unordered_map<std::string, PhongMaterial> mtl;
	SubMesh* submesh = nullptr;
	float x = 0, y = 0, z = 0;

	if (!file.is_open()) {
		std::cout << "fail to open file\n";
		return false;
	}

	while (file >> s) {
		if (s == "mtllib") {
			file >> s;
			std::ifstream mtlFile(GetParentPath(filePath) + s, std::ios::in);
			if (!mtlFile.is_open()) {
				std::cout << "fail" << std::endl;
				return false;
			}
			std::string mtlName;
			PhongMaterial pmtl;
			while (mtlFile >> s) {
				if (s == "newmtl") {
					mtl[mtlName].SetName(pmtl.GetName());
					mtl[mtlName].SetNs(pmtl.GetNs());
					mtl[mtlName].SetKa(pmtl.GetKa());
					mtl[mtlName].SetKd(pmtl.GetKd());
					mtl[mtlName].SetKs(pmtl.GetKs());
					mtl[mtlName].SetMapKd(pmtl.GetMapKd());
					mtlFile >> mtlName;
					pmtl.SetName(mtlName);
				}
				if (s == "Ns") {
					float x;
					mtlFile >> x;
					pmtl.SetNs(x);
				}
				if (s == "Ka") {
					float x, y, z;
					mtlFile >> x >> y >> z;
					glm::vec3 Ka(x, y, z);
					pmtl.SetKa(Ka);
				}
				if (s == "Kd") {
					float x, y, z;
					mtlFile >> x >> y >> z;
					glm::vec3 Kd(x, y, z);
					pmtl.SetKd(Kd);
				}
				if (s == "Ks") {
					float x, y, z;
					mtlFile >> x >> y >> z;
					glm::vec3 Ks(x, y, z);
					pmtl.SetKs(Ks);
				}
				if (s == "map_Kd") {
					mtlFile >> s;
					ImageTexture* tmpTex = new ImageTexture(GetParentPath(filePath) + s);
					pmtl.SetMapKd(tmpTex);
				}
			}
		}
		if (s == "v") {
			file >> x >> y >> z;
			glm::vec3 p(x, y, z);
			positions.push_back(p);
		}
		if (s == "vn") {
			file >> x >> y >> z;
			glm::vec3 n(x, y, z);
			normals.push_back(n);
		}
		if (s == "vt") {
			file >> x >> y;
			glm::vec2 t(x, y);
			texcoords.push_back(t);
		}
		if (s == "usemtl") {
			SubMeshFaceData.push_back(SingleSubMeshFaceData);
			SingleSubMeshFaceData.clear();
			file >> s;
			MaterialName.push_back(s);
		}
		if (s == "f") {
			std::getline(file, s);
			s.erase(s.begin());
			std::vector<std::string> v = split(s, ' ');
			std::stringstream ss;
			std::string p, t, n;
			if (v.size() == 3) {
				for (int i = 0; i < 3; i++) {
					ss << v[i];
					std::getline(ss, p, '/');
					std::getline(ss, t, '/');
					std::getline(ss, n, '/');
					SingleSubMeshFaceData.push_back({ stoi(p), stoi(t), stoi(n) });
					ss.clear();
				}
				numTriangles++;
			}
			else {
				for (int i = 1; i < v.size() - 1; i++) {
					ss.str(v[0]);
					std::getline(ss, p, '/');
					std::getline(ss, t, '/');
					std::getline(ss, n, '/');
					SingleSubMeshFaceData.push_back({ stoi(p), stoi(t), stoi(n) });
					ss.clear();
					for (int j = 0; j < 2; j++) {
						ss.str(v[i + j]);
						std::getline(ss, p, '/');
						std::getline(ss, t, '/');
						std::getline(ss, n, '/');
						SingleSubMeshFaceData.push_back({ stoi(p), stoi(t), stoi(n) });
						ss.clear();
					}
					numTriangles++;
				}
			}
		}

	}
	// Normalize the geometry data.
	if (normalized) {
		glm::vec3 minV(FLT_MAX, FLT_MAX, FLT_MAX), maxV(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (auto i : positions) {
			minV.x = std::min(i.x, minV.x);
			minV.y = std::min(i.y, minV.y);
			minV.z = std::min(i.z, minV.z);
			maxV.x = std::max(i.x, maxV.x);
			maxV.y = std::max(i.y, maxV.y);
			maxV.z = std::max(i.z, maxV.z);
		}
		objExtent = glm::vec3(maxV.x - minV.x, maxV.y - minV.y, maxV.z - minV.z);
		objCenter = glm::vec3((minV.x + maxV.x) / 2, (minV.y + maxV.y) / 2, (minV.z + maxV.z) / 2);
		float max_extent = std::max(maxV.x - minV.x, maxV.y - minV.y);
		max_extent = std::max(max_extent, maxV.z - minV.z);
		objExtent = objExtent / glm::vec3(max_extent);
		for (int i = 0; i < positions.size(); i++) {
			positions[i].x -= objCenter.x;
			positions[i].y -= objCenter.y;
			positions[i].z -= objCenter.z;

			positions[i].x /= max_extent;
			positions[i].y /= max_extent;
			positions[i].z /= max_extent;
		}
		objCenter = glm::vec3(0, 0, 0);
	}
	SubMeshFaceData.push_back(SingleSubMeshFaceData);
	SubMeshFaceData.erase(SubMeshFaceData.begin());
	for (int i = 0; i < SubMeshFaceData.size(); i++) {
		SubMesh sub;
		sub.material = new PhongMaterial;
		sub.material->SetName(MaterialName[i]);
		sub.material->SetKa(mtl[MaterialName[i]].GetKa());
		sub.material->SetKd(mtl[MaterialName[i]].GetKd());
		sub.material->SetKs(mtl[MaterialName[i]].GetKs());
		sub.material->SetMapKd(mtl[MaterialName[i]].GetMapKd());

		for (int j = 0; j < SubMeshFaceData[i].size(); j++) {
			glm::vec3 info = SubMeshFaceData[i][j] - glm::vec3(1, 1, 1);
			VertexPTN vtx;
			vtx.position = positions[info.x];
			vtx.texcoord = texcoords[info.y];
			vtx.normal = normals[info.z];
			unsigned int vtxidx = GetVertexIndex(vtx);
			if (vtxidx == vertices.size()) {
				if (vertices.size() == 0) {
					vtxidx = 0;
				}
				vertices.push_back(vtx);
			}
			sub.vertexIndices.push_back(vtxidx);
		}
		subMeshes.push_back(sub);
	}
	numVertices = vertices.size();
	return true;
}

unsigned int TriangleMesh::GetVertexIndex(VertexPTN vtx) {
	for (int i = 0; i < vertices.size(); i++) {
		if (vertices[i].position == vtx.position && vertices[i].normal == vtx.normal && vertices[i].texcoord == vtx.texcoord)
			return i;
	}
	return vertices.size();
}

void TriangleMesh::CreateBuffers() {
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPTN) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	for (auto& i : subMeshes) {
		glGenBuffers(1, &(i.iboId));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i.iboId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * i.vertexIndices.size(), &(i.vertexIndices[0]), GL_STATIC_DRAW);
	}
}

void TriangleMesh::Render(PhongShadingDemoShaderProg* phongShadingShader) {
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (const GLvoid*)12);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (const GLvoid*)24);

	for (auto& i : subMeshes) {
		// Material properties.
		glUniform3fv(phongShadingShader->GetLocKa(), 1, glm::value_ptr(i.material->GetKa()));
		glUniform3fv(phongShadingShader->GetLocKd(), 1, glm::value_ptr(i.material->GetKd()));
		glUniform3fv(phongShadingShader->GetLocKs(), 1, glm::value_ptr(i.material->GetKs()));
		glUniform1f(phongShadingShader->GetLocNs(), i.material->GetNs());
		if (i.material->GetMapKd() != nullptr) {
			i.material->GetMapKd()->Bind(GL_TEXTURE0);
			glUniform1i(phongShadingShader->GetLocMapKd(), 0);
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i.iboId);
		glDrawElements(GL_TRIANGLES, (GLsizei)(i.vertexIndices.size()), GL_UNSIGNED_INT, 0);
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
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

