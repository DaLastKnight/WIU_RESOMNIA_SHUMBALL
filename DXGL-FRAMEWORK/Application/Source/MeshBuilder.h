#ifndef MESH_BUILDER_H
#define MESH_BUILDER_H

#include "Mesh.h"
#include "Vertex.h"
#include "LoadOBJ.h"

/******************************************************************************/
/*!
		Class MeshBuilder:
\brief	Provides methods to generate mesh of different shapes
*/
/******************************************************************************/
class MeshBuilder
{
public:
	static Mesh* GenerateAxes(const std::string &meshName, float lengthX, float lengthY, float lengthZ);
	static Mesh* GenerateSphere(const std::string& meshName, glm::vec3 color, float radius = 1.f, int numSlice = 360, int numStack = 360);
	static Mesh* GenerateCone(const std::string& meshName, glm::vec3 color, int numSlice, float radius, float height, float bottomOffset = 0);
	static Mesh* GenerateTorus(const std::string& meshName, glm::vec3 color, float innerR = 1.f, float outerR = 1.f, float numSlice = 360, float numStack = 360);
	static Mesh* GenerateFrustum(const std::string& meshName, glm::vec3 color, float baseRadius, float topRadius, float height, float numSlice = 360);
	static Mesh* GenerateQuad(const std::string& meshName, glm::vec3 color, float width, float height);
	static Mesh* GenerateCube(const std::string& meshName, glm::vec3 color, float length);
	static Mesh* GenerateTrap(const std::string& meshName, glm::vec3 color, float baseLength, float baseWidth, float topLength, float topWidth, float height);
	static Mesh* GenerateHemisphere(const std::string& meshName, glm::vec3 color, int numStack, int numSlice, float radius);
	static Mesh* GenerateCylinder(const std::string& meshName, glm::vec3 color, int numSlice, float radius, float height);

	static Mesh* GenerateOBJ(const std::string& meshName, const std::string& file_path, bool flipUVs = false);
	static Mesh* GenerateOBJMTL(const std::string& meshName, const std::string& file_path, const std::string& mtl_path, bool flipUVs = false);

	static Mesh* GenerateText(const std::string& meshName, unsigned numRow, unsigned numCol, float charSpacing);

	static Mesh* GenerateSkybox(const std::string& meshName);
};

#endif