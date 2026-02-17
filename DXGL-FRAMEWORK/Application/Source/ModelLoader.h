#ifndef LOBJ_LOADER_H
#define LOBJ_LOADER_H

#include <string>
#include <vector>
#include <map>
#include <glm\glm.hpp>

#include "Vertex.h"
#include "Material.h"

class ModelLoader {

	static std::string directory;

	struct PackedVertex {
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		bool operator<(const PackedVertex that) const {
			return memcmp((void*)this, (void*)&that, sizeof(PackedVertex)) > 0;
		};
	};

	static bool getSimilarVertexIndex_fast(
		PackedVertex& packed,
		std::map<PackedVertex, unsigned short>& VertexToOutIndex,
		unsigned short& result
	);

	static bool LoadMTL(
		const char* file_path,
		std::map<std::string,
		Material*>& materials_map);

public:

	static void setDirectory(const std::string& directoryPath);

	static bool LoadOBJ(
		const char* file_path,
		std::vector<glm::vec3>& out_vertices,
		std::vector<glm::vec2>& out_uvs,
		std::vector<glm::vec3>& out_normals
	);

	static void IndexVBO(
		std::vector<glm::vec3>& in_vertices,
		std::vector<glm::vec2>& in_uvs,
		std::vector<glm::vec3>& in_normals,

		std::vector<unsigned>& out_indices,
		std::vector<Vertex>& out_vertices
	);

	static bool LoadOBJMTL(
		const char* file_path,
		const char* mtl_path,
		std::vector<glm::vec3>& out_vertices,
		std::vector<glm::vec2>& out_uvs,
		std::vector<glm::vec3>& out_normals,
		std::vector<Material>& out_materials
	);
};

#endif