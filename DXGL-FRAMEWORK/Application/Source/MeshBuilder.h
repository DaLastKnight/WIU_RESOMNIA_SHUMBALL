#ifndef MESH_BUILDER_H
#define MESH_BUILDER_H

#include "Mesh.h"
#include "Vertex.h"
#include "ModelLoader.h"

namespace reactphysics3d {
	class DebugRenderer;
}

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
	// mode | 0 = normal render, 1 = inverse render
	static Mesh* GenerateSphere(const std::string& meshName, glm::vec3 color = glm::vec3(1.f), float radius = 1.f, unsigned int slices = 16, unsigned int stacks = 8, int textureID = -1, unsigned int mode = 0);
	static Mesh* GenerateCone(const std::string& meshName, glm::vec3 color, int numSlice, float radius, float height, float bottomOffset = 0);
	// mode | 0 = normal render, 1 = inverse render
	static Mesh* GenerateTorus(const std::string& meshName, glm::vec3 color = glm::vec3(1.f), float width = 1.f, float radius = 1.f, unsigned int slices = 16, unsigned int stacks = 16, unsigned int mode = 0);
	static Mesh* GenerateFrustum(const std::string& meshName, glm::vec3 color, float baseRadius, float topRadius, float height, float numSlice = 360);
	static Mesh* GenerateQuad(const std::string& meshName, glm::vec3 color, float width, float height, int textureID = -1);
	static Mesh* GenerateCube(const std::string& meshName, glm::vec3 color, float length);
	static Mesh* GenerateTrap(const std::string& meshName, glm::vec3 color, float baseLength, float baseWidth, float topLength, float topWidth, float height);
	static Mesh* GenerateHemisphere(const std::string& meshName, glm::vec3 color, int numStack, int numSlice, float radius);
	static Mesh* GenerateCylinder(const std::string& meshName, glm::vec3 color, int numSlice, float radius, float height);

	static Mesh* GenerateOBJ(const std::string& meshName, const std::string& file_path, int textureID = -1);
	static Mesh* GenerateOBJMTL(const std::string& meshName, const std::string& file_path, const std::string& mtl_path, int textureID = -1);

	static Mesh* GenerateText(const std::string& meshName, unsigned numRow, unsigned numCol, float advanceWidth, unsigned textureID);

	static Mesh* GenerateSkybox(const std::string& meshName, unsigned textureID);
	static Mesh* GenerateGround(const std::string& meshName, float size, float texSize, unsigned textureID);

	static Mesh* GenerateLine(const std::string& meshName, float length);

	static Mesh* GenratePhysicsWorld(const reactphysics3d::DebugRenderer* debugRenderer);
};

#endif