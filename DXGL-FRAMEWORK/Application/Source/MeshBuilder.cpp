#include "MeshBuilder.h"
#include <GL\glew.h>
#include <vector>

#include "glm\glm.hpp"
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

/******************************************************************************/
/*!
\brief
Generate the vertices of a reference Axes; Use red for x-axis, green for y-axis, blue for z-axis
Then generate the VBO/IBO and store them in Mesh object

\param meshName - name of mesh
\param lengthX - x-axis should start at -lengthX / 2 and end at lengthX / 2
\param lengthY - y-axis should start at -lengthY / 2 and end at lengthY / 2
\param lengthZ - z-axis should start at -lengthZ / 2 and end at lengthZ / 2

\return Pointer to mesh storing VBO/IBO of reference axes
*/
/******************************************************************************/
Mesh* MeshBuilder::GenerateAxes(const std::string &meshName, float lengthX, float lengthY, float lengthZ)
{
	Vertex v;
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	//x-axis
	v.pos = glm::vec3(-lengthX, 0, 0);	v.color = glm::vec3(1, 0, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(lengthX, 0, 0);	v.color = glm::vec3(1, 0, 0);	vertex_buffer_data.push_back(v);
	//y-axis
	v.pos = glm::vec3(0, -lengthY, 0);	v.color = glm::vec3(0, 1, 0);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0, lengthY, 0);	v.color = glm::vec3(0, 1, 0);	vertex_buffer_data.push_back(v);
	//z-axis
	v.pos = glm::vec3(0, 0, -lengthZ);	v.color = glm::vec3(0, 0, 1);	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0, 0, lengthZ);	v.color = glm::vec3(0, 0, 1);	vertex_buffer_data.push_back(v);

	index_buffer_data.push_back(0);
	index_buffer_data.push_back(1);
	index_buffer_data.push_back(2);
	index_buffer_data.push_back(3);
	index_buffer_data.push_back(4);
	index_buffer_data.push_back(5);

	Mesh *mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_LINES;

	return mesh;
}

/******************************************************************************/
/*!
\brief
Generate the vertices of a quad; Use random color for each vertex
Then generate the VBO/IBO and store them in Mesh object

\param meshName - name of mesh
\param lengthX - width of quad
\param lengthY - height of quad

\return Pointer to mesh storing VBO/IBO of quad
*/
/******************************************************************************/

Mesh* MeshBuilder::GenerateSphere(const std::string& meshName, glm::vec3 color, float radius, unsigned int slices, unsigned int stacks, unsigned int mode)
{
	Vertex v; // Vertex definition
	std::vector<Vertex> vertex_buffer_data; // Vertex Buffer Objects
	std::vector<GLuint> index_buffer_data; // Element Buffer Objects
	v.color = color;

	float degreePerStack = glm::pi<float>() / stacks;
	float degreePerSlice = glm::two_pi<float>() / slices;

	unsigned reserveCount = 0;
	reserveCount = (stacks + 1) * (slices + 1);
	vertex_buffer_data.reserve(reserveCount);

	for (unsigned stack = 0; stack < stacks + 1; ++stack) //stack
	{
		float phi = -glm::half_pi<float>() + stack * degreePerStack;
		for (unsigned slice = 0; slice < slices + 1; ++slice) //slice
		{
			float theta = slice * degreePerSlice;

			v.pos = glm::vec3(radius * glm::cos(phi) * glm::cos(theta),
				radius * glm::sin(phi),
				radius * glm::cos(phi) * glm::sin(theta));
			v.normal = glm::vec3(glm::cos(phi) * glm::cos(theta),
				glm::sin(phi),
				glm::cos(phi) * glm::sin(theta));
			float uCoord = static_cast<float>(slice) / slices;
			float vCoord = static_cast<float>(stack) / stacks;
			v.texCoord = glm::vec2(uCoord, vCoord);

			vertex_buffer_data.push_back(v);
		}
	}

	reserveCount = (stacks) * (slices + 1);
	index_buffer_data.reserve(reserveCount * 2);

	for (unsigned stack = 0; stack < stacks; ++stack) {
		for (unsigned slice = 0; slice < slices + 1; ++slice) {
			if (mode == 1) {
				index_buffer_data.push_back(slice + stack * (slices + 1) + slices);
				index_buffer_data.push_back(slice + stack * (slices + 1));
			}
			else {
				index_buffer_data.push_back(slice + stack * (slices + 1));
				index_buffer_data.push_back(slice + (stack + 1) * (slices + 1));
			}
		}
	}

	// Create the new mesh
	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLE_STRIP;

	return mesh;
}

Mesh* MeshBuilder::GenerateCone(const std::string& meshName, glm::vec3 color, int numSlice, float radius, float height, float bottomOffset)
{
	Vertex v, temp, coneHypo, coneHeight, center;
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	v.color = color;
	float degreePerSlice = glm::two_pi<float>() / numSlice;
	coneHeight.pos = glm::vec3(0.f, 0.5f * height, 0.f);

	// Cone tip vertex
	int tipIndex = 0;
	for (int i = 0; i < 2; i++)
	{
		for (unsigned slice = 0; slice < numSlice + 1; ++slice)
		{
			float theta = slice * degreePerSlice;

			if (i == 0)
			{
				v.pos = glm::vec3(0.f, 0.5f * height, 0.f);
				temp.pos = glm::vec3(radius * cos(theta), height * -0.5f, radius * sin(theta)); // Used to calc normal
			}
			else
			{
				temp.pos = v.pos = glm::vec3(radius * cos(theta), height * -0.5f, radius * sin(theta));
			}

			coneHypo.pos = coneHeight.pos - temp.pos;
			v.color = color;
			v.normal = glm::normalize(coneHeight.pos - (glm::dot(coneHeight.pos, coneHypo.pos)) / (glm::dot(coneHypo.pos, coneHypo.pos)) * coneHypo.pos);
			vertex_buffer_data.push_back(v);
		}
	}

	
	// Bottom surface and center (Typical cone structure)
	for (int i = 0; i < 2; i++)
	{
		for (unsigned slice = 0; slice < numSlice + 1; ++slice)
		{
			if (i == 0)
			{
				float theta = slice * degreePerSlice;
				v.pos = glm::vec3(radius * cos(theta), -0.5f * height, radius * sin(theta));
				v.normal = glm::vec3(0.f, -1.f, 0.f);
				vertex_buffer_data.push_back(v);
			}
			else
			{
				center.pos = glm::vec3(0.f, -0.5f * height, 0.f);
				center.color = color;
				center.normal = glm::vec3(0.f, -1.f, 0.f);
				vertex_buffer_data.push_back(center);
			}
		}
	}
	
	
	
	// Custom cone
	/*
	coneHeight.pos = glm::vec3(0.f, -0.5f * height, 0.f);
	for (int i = 0; i < 2; i++)
	{
		for (unsigned slice = 0; slice < numSlice + 1; ++slice)
		{
			float theta = slice * degreePerSlice;

			if (i == 0)
			{
				temp.pos = v.pos = glm::vec3(radius * cos(theta), height * -0.5f, radius * sin(theta));
				
			}
			else
			{
				v.pos = glm::vec3(0.f, bottomOffset - 0.5f * height, 0.f);
				temp.pos = glm::vec3(radius * cos(theta), height * -0.5f, radius * sin(theta)); // Used to calc normal
			}

			coneHypo.pos = coneHeight.pos - temp.pos;
			v.color = color;
			v.normal = glm::normalize(coneHeight.pos - (glm::dot(coneHeight.pos, coneHypo.pos)) / (glm::dot(coneHypo.pos, coneHypo.pos)) * coneHypo.pos);
			vertex_buffer_data.push_back(v);
		}
	}
	*/
	
	
	for (int i = 0; i < 4; i++)
	{
		for (int slice = 0; slice < numSlice + 1; ++slice)
		{
			index_buffer_data.push_back(slice + i * (numSlice + 1) + numSlice);
			index_buffer_data.push_back(slice + i * (numSlice + 1));
		}
	}

	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLE_STRIP;

	return mesh;
}

Mesh* MeshBuilder::GenerateTorus(const std::string& meshName, glm::vec3 color, float width, float radius, unsigned int slices, unsigned int stacks, unsigned int mode) {

	Vertex v; // Vertex definition
	std::vector<Vertex> vertex_buffer_data; // Vertex Buffer Objects
	std::vector<GLuint> index_buffer_data; // Element Buffer Objects
	v.color = color;

	width /= 2;

	float degreePerStack = glm::two_pi<float>() / stacks;
	float degreePerSlice = glm::two_pi<float>() / slices;

	unsigned reserveCount = 0;
	reserveCount = (stacks + 1) * (slices + 1);
	vertex_buffer_data.reserve(reserveCount);

	for (unsigned stack = 0; stack < stacks + 1; ++stack) //stack
	{
		float phi = stack * degreePerStack;
		for (unsigned slice = 0; slice < slices + 1; ++slice) //slice
		{
			float theta = slice * degreePerSlice;

			v.pos = glm::vec3((radius + width * glm::cos(theta)) * glm::sin(phi),
				width * glm::sin(theta),
				(radius + width * glm::cos(theta)) * glm::cos(phi));
			v.normal = glm::vec3(glm::sin(phi) * glm::cos(theta),
				glm::sin(phi),
				glm::cos(phi) * glm::sin(theta));
			float uCoord = static_cast<float>(slice) / slices;
			float vCoord = static_cast<float>(stack) / stacks;
			v.texCoord = glm::vec2(uCoord, vCoord);

			vertex_buffer_data.push_back(v);
		}
	}

	reserveCount = (stacks) * (slices + 1);
	index_buffer_data.reserve(reserveCount * 2);

	for (unsigned stack = 0; stack < stacks; ++stack) {
		for (unsigned slice = 0; slice < slices + 1; ++slice) {
			if (mode) {
				index_buffer_data.push_back(slice + (stack) * (slices + 1) + slices);
				index_buffer_data.push_back(slice + stack * (slices + 1));
			}
			else {
				index_buffer_data.push_back(slice + stack * (slices + 1));
				index_buffer_data.push_back(slice + (stack + 1) * (slices + 1));
			}
		}
	}

	// Create the new mesh
	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLE_STRIP;

	return mesh;

}

Mesh* MeshBuilder::GenerateFrustum(const std::string& meshName, glm::vec3 color, float baseRadius, float topRadius, float height, float numSlice)
{
	Vertex v, temp, frustHypo, frustHeight;
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	v.color = color;

	float degreePerSlice = glm::two_pi<float>() / numSlice;
	
	// Bottom flat surface
	v.pos = glm::vec3(0.f, -0.5f * height, 0);
	v.normal = glm::vec3(0.f, -1.f, 0.f);
	vertex_buffer_data.push_back(v);
	for (unsigned slice = 0; slice < numSlice + 1; ++slice)
	{
		float theta = slice * degreePerSlice;
		v.pos = glm::vec3(baseRadius * cos(theta), -0.5f * height, baseRadius * sin(theta));
		v.normal = glm::vec3(0.f, -1.f, 0.f);
		vertex_buffer_data.push_back(v);
	}

	for (int slice = 0; slice < numSlice + 1; ++slice)
	{
		index_buffer_data.push_back(0);
		index_buffer_data.push_back(slice + 1);
	}
	
	// Middle
	frustHeight.pos = glm::vec3(0.f, 0.5f * height, 0.f);
	unsigned middleStartIndex = vertex_buffer_data.size();
	for (int slice = 0; slice < numSlice + 1; ++slice)
	{
		float theta = slice * degreePerSlice;
		v.pos = temp.pos = glm::vec3(baseRadius * cos(theta), height * -0.5f, baseRadius * sin(theta));
		frustHypo.pos = frustHeight.pos - temp.pos;
		v.normal = glm::normalize(frustHeight.pos - (glm::dot(frustHeight.pos, frustHypo.pos)) / (glm::dot(frustHypo.pos, frustHypo.pos)) * frustHypo.pos);
		//v.normal = glm::vec3(glm::cos(theta), 0.f, glm::sin(theta));
		vertex_buffer_data.push_back(v);

		v.pos = glm::vec3(topRadius * cos(theta), height * 0.5f, topRadius * sin(theta));
		vertex_buffer_data.push_back(v);
	}

	for (unsigned slice = 0; slice < numSlice + 1; slice++)
	{
		index_buffer_data.push_back(middleStartIndex + 2 * slice + 0);
		index_buffer_data.push_back(middleStartIndex + 1 + 2 * slice + 0);
	}

	// Top flat surface
	unsigned topStartIndex = vertex_buffer_data.size();
	v.pos = glm::vec3(0, 0.5f * height, 0);
	v.normal = glm::vec3(0.f, 1.f, 0.f);
	vertex_buffer_data.push_back(v);

	for (unsigned slice = 0; slice < numSlice + 1; ++slice)
	{
		float theta = slice * degreePerSlice;
		v.pos = glm::vec3(topRadius * cos(theta), 0.5f * height, topRadius * sin(theta));
		v.normal = glm::vec3(0.f, 1.f, 0.f);
		vertex_buffer_data.push_back(v);
	}

	for (int slice = 0; slice < numSlice + 1; ++slice)
	{
		index_buffer_data.push_back(topStartIndex + slice + 1);
		index_buffer_data.push_back(topStartIndex);
	}
	

	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLE_STRIP;

	return mesh;
}

Mesh* MeshBuilder::GenerateQuad(const std::string& meshName, glm::vec3 color, float width, float height, int textureID)
{
	Vertex v;
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	v.color = color;
	v.normal = glm::vec3(0.f, 0.f, 1.f);

	// Add the vertices here
	v.pos = glm::vec3(0.5f * width, -0.5f * height, 0.f); // Bottom right
	v.texCoord = glm::vec2(1.f, 0.f);
	vertex_buffer_data.push_back(v); //v3
	v.pos = glm::vec3(0.5f * width, 0.5f * height, 0.f); // Top right
	v.texCoord = glm::vec2(1.f, 1.f);
	vertex_buffer_data.push_back(v); //v0
	v.pos = glm::vec3(-0.5f * width, 0.5f * height, 0.f); // Top left
	v.texCoord = glm::vec2(0.f, 1.f);
	vertex_buffer_data.push_back(v); //v1
	v.pos = glm::vec3(-0.5f * width, -0.5f * height, 0.f); // Bottom left
	v.texCoord = glm::vec2(0.f, 0.f);
	vertex_buffer_data.push_back(v); //v2

	//tri1
	index_buffer_data.push_back(0);
	index_buffer_data.push_back(1);
	index_buffer_data.push_back(2);
	//tri2
	index_buffer_data.push_back(0);
	index_buffer_data.push_back(2);
	index_buffer_data.push_back(3);

	for (unsigned i = 0; i < 36; ++i)
	{
		index_buffer_data.push_back(i);
	}

	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLES;
	if (textureID != -1)
		mesh->textureID = textureID;

	return mesh;
}

Mesh* MeshBuilder::GenerateCube(const std::string& meshName, glm::vec3 color, float length)
{
	Vertex v;
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	v.color = color;

	// Front
	v.pos = glm::vec3(0.5f * length, 0.5f * length, 0.5f * length);
	v.normal = glm::vec3(0.f, 0.f, 1.f);
	v.texCoord = glm::vec2(1.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, 0.5f * length);
	v.normal = glm::vec3(0.f, 0.f, 1.f);
	v.texCoord = glm::vec2(0.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, 0.5f * length);
	v.normal = glm::vec3(0.f, 0.f, 1.f);
	v.texCoord = glm::vec2(0.f, 0.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, 0.5f * length, 0.5f * length);
	v.normal = glm::vec3(0.f, 0.f, 1.f);
	v.texCoord = glm::vec2(1.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, 0.5f * length);
	v.normal = glm::vec3(0.f, 0.f, 1.f);
	v.texCoord = glm::vec2(0.f, 0.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, 0.5f * length);
	v.normal = glm::vec3(0.f, 0.f, 1.f);
	v.texCoord = glm::vec2(1.f, 0.f);
	vertex_buffer_data.push_back(v);

	// Right
	v.pos = glm::vec3(0.5f * length, 0.5f * length, -0.5f * length);
	v.normal = glm::vec3(1.f, 0.f, 0.f);
	v.texCoord = glm::vec2(1.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, 0.5f * length, 0.5f * length);
	v.normal = glm::vec3(1.f, 0.f, 0.f);
	v.texCoord = glm::vec2(0.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, 0.5f * length);
	v.normal = glm::vec3(1.f, 0.f, 0.f);
	v.texCoord = glm::vec2(0.f, 0.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, 0.5f * length, -0.5f * length);
	v.normal = glm::vec3(1.f, 0.f, 0.f);
	v.texCoord = glm::vec2(1.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, 0.5f * length);
	v.normal = glm::vec3(1.f, 0.f, 0.f);
	v.texCoord = glm::vec2(0.f, 0.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, -0.5f * length);
	v.normal = glm::vec3(1.f, 0.f, 0.f);
	v.texCoord = glm::vec2(1.f, 0.f);
	vertex_buffer_data.push_back(v);

	// Top
	v.pos = glm::vec3(0.5f * length, 0.5f * length, -0.5f * length);
	v.normal = glm::vec3(0.f, 1.f, 0.f);
	v.texCoord = glm::vec2(1.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, -0.5f * length);
	v.normal = glm::vec3(0.f, 1.f, 0.f);
	v.texCoord = glm::vec2(0.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, 0.5f * length);
	v.normal = glm::vec3(0.f, 1.f, 0.f);
	v.texCoord = glm::vec2(0.f, 0.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, 0.5f * length, -0.5f * length);
	v.normal = glm::vec3(0.f, 1.f, 0.f);
	v.texCoord = glm::vec2(1.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, 0.5f * length);
	v.normal = glm::vec3(0.f, 1.f, 0.f);
	v.texCoord = glm::vec2(0.f, 0.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, 0.5f * length, 0.5f * length);
	v.normal = glm::vec3(0.f, 1.f, 0.f);
	v.texCoord = glm::vec2(1.f, 0.f);
	vertex_buffer_data.push_back(v);

	// Back
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, -0.5f * length);
	v.normal = glm::vec3(0.f, 0.f, -1.f);
	v.texCoord = glm::vec2(1.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, 0.5f * length, -0.5f * length);
	v.normal = glm::vec3(0.f, 0.f, -1.f);
	v.texCoord = glm::vec2(0.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, -0.5f * length);
	v.normal = glm::vec3(0.f, 0.f, -1.f);
	v.texCoord = glm::vec2(0.f, 0.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, -0.5f * length);
	v.normal = glm::vec3(0.f, 0.f, -1.f);
	v.texCoord = glm::vec2(1.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, -0.5f * length);
	v.normal = glm::vec3(0.f, 0.f, -1.f);
	v.texCoord = glm::vec2(0.f, 0.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, -0.5f * length);
	v.normal = glm::vec3(0.f, 0.f, -1.f);
	v.texCoord = glm::vec2(1.f, 0.f);
	vertex_buffer_data.push_back(v);

	// Left
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, -0.5f * length);
	v.normal = glm::vec3(-1.f, 0.f, 0.f);
	v.texCoord = glm::vec2(1.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, 0.5f * length);
	v.normal = glm::vec3(-1.f, 0.f, 0.f);
	v.texCoord = glm::vec2(0.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, -0.5f * length);
	v.normal = glm::vec3(-1.f, 0.f, 0.f);
	v.texCoord = glm::vec2(0.f, 0.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, 0.5f * length, 0.5f * length);
	v.normal = glm::vec3(-1.f, 0.f, 0.f);
	v.texCoord = glm::vec2(1.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, -0.5f * length);
	v.normal = glm::vec3(-1.f, 0.f, 0.f);
	v.texCoord = glm::vec2(0.f, 0.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, 0.5f * length);
	v.normal = glm::vec3(-1.f, 0.f, 0.f);
	v.texCoord = glm::vec2(1.f, 0.f);
	vertex_buffer_data.push_back(v);

	// Bottom
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, -0.5f * length);
	v.normal = glm::vec3(0.f, -1.f, 0.f);
	v.texCoord = glm::vec2(1.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, -0.5f * length);
	v.normal = glm::vec3(0.f, -1.f, 0.f);
	v.texCoord = glm::vec2(0.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, 0.5f * length);
	v.normal = glm::vec3(0.f, -1.f, 0.f);
	v.texCoord = glm::vec2(0.f, 0.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, -0.5f * length);
	v.normal = glm::vec3(0.f, -1.f, 0.f);
	v.texCoord = glm::vec2(1.f, 1.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * length, -0.5f * length, 0.5f * length);
	v.normal = glm::vec3(0.f, -1.f, 0.f);
	v.texCoord = glm::vec2(0.f, 0.f);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * length, -0.5f * length, 0.5f * length);
	v.normal = glm::vec3(0.f, -1.f, 0.f);
	v.texCoord = glm::vec2(1.f, 0.f);
	vertex_buffer_data.push_back(v);

	for (unsigned i = 0; i < 36; ++i)
	{
		index_buffer_data.push_back(i);
	}

	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLES;

	return mesh;
}

Mesh* MeshBuilder::GenerateTrap(const std::string& meshName, glm::vec3 color, float baseLength, float baseWidth, float topLength, float topWidth, float height)
{
	Vertex v;
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	v.color = color;
	
	// Front
	float angle = atanf(0.5f * (baseWidth - topWidth) / height);
	v.normal = glm::vec3(0.f, sin(angle), cos(angle));
	v.pos = glm::vec3(-0.5f * topLength, 0.5f * height, 0.5f * topWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * baseLength, -0.5f * height, 0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * baseLength, -0.5f * height, 0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * topLength, 0.5f * height, 0.5f * topWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * baseLength, -0.5f * height, 0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * topLength, 0.5f * height, 0.5f * topWidth);
	vertex_buffer_data.push_back(v);

	// Right
	angle = atanf(0.5f * (baseLength - topLength) / height);
	v.normal = glm::vec3(cos(angle), sin(angle), 0.f);
	v.pos = glm::vec3(0.5f * topLength, 0.5f * height, 0.5f * topWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * baseLength, -0.5f * height, 0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * baseLength, -0.5f * height, -0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * topLength, 0.5f * height, 0.5f * topWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * baseLength, -0.5f * height, -0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * topLength, 0.5f * height, -0.5f * topWidth);
	vertex_buffer_data.push_back(v);

	// Back
	angle = atanf(0.5f * (baseWidth - topWidth) / height);
	v.normal = glm::vec3(0.f, sin(angle), -cos(angle));
	v.pos = glm::vec3(0.5f * topLength, 0.5f * height, -0.5f * topWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * baseLength, -0.5f * height, -0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * baseLength, -0.5f * height, -0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * topLength, 0.5f * height, -0.5f * topWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * baseLength, -0.5f * height, -0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * topLength, 0.5f * height, -0.5f * topWidth);
	vertex_buffer_data.push_back(v);

	// Left
	angle = atanf(0.5f * (baseLength - topLength) / height);
	v.normal = glm::vec3(-cos(angle), sin(angle), 0.f);
	v.pos = glm::vec3(-0.5f * topLength, 0.5f * height, -0.5f * topWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * baseLength, -0.5f * height, -0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * baseLength, -0.5f * height, 0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * topLength, 0.5f * height, -0.5f * topWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * baseLength, -0.5f * height, 0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * topLength, 0.5f * height, 0.5f * topWidth);
	vertex_buffer_data.push_back(v);

	// Top
	v.normal = glm::vec3(0.f, 1.f, 0.f);
	v.pos = glm::vec3(-0.5f * topLength, 0.5f * height, -0.5f * topWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * topLength, 0.5f * height, 0.5f * topWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * topLength, 0.5f * height, 0.5f * topWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * topLength, 0.5f * height, -0.5f * topWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * topLength, 0.5f * height, 0.5f * topWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * topLength, 0.5f * height, 0.5f * -topWidth);
	vertex_buffer_data.push_back(v);

	// Bottom
	v.normal = glm::vec3(0.f, -1.f, 0.f);
	v.pos = glm::vec3(0.5f * baseLength, -0.5f * height, 0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * baseLength, -0.5f * height, 0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * baseLength, -0.5f * height, -0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(-0.5f * baseLength, -0.5f * height, -0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * baseLength, -0.5f * height, -0.5f * baseWidth);
	vertex_buffer_data.push_back(v);
	v.pos = glm::vec3(0.5f * baseLength, -0.5f * height, 0.5f * baseWidth);
	vertex_buffer_data.push_back(v);

	for (unsigned i = 0; i < 36; ++i)
	{
		index_buffer_data.push_back(i);
	}
	
	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLES;

	return mesh;
}

Mesh* MeshBuilder::GenerateHemisphere(const std::string& meshName, glm::vec3 color, int numStack, int numSlice, float radius)
{
	Vertex v;
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	v.color = color;

	const float degreePerStack = glm::half_pi<float>() / numStack;
	const float degreePerSlice = glm::two_pi<float>() / numSlice;

	// Top surface
	v.pos = glm::vec3(0, 0, 0);
	v.normal = glm::vec3(0.f, -1.f, 0.f);
	vertex_buffer_data.push_back(v);

	for (unsigned slice = 0; slice < numSlice + 1; ++slice)
	{
		float theta = slice * degreePerSlice;
		v.pos = glm::vec3(radius * cos(theta), 0, radius * sin(theta));
		v.normal = glm::vec3(0.f, -1.f, 0.f);
		vertex_buffer_data.push_back(v);
	}

	for (unsigned slice = 0; slice < numSlice + 1; ++slice)
	{
		index_buffer_data.push_back(0);
		index_buffer_data.push_back(slice + 1);
	}


	unsigned hemisphereStart = vertex_buffer_data.size();
	for (unsigned stack = 0; stack < numStack + 1; ++stack)
	{
		float phi = 0.f + stack * degreePerStack;
		for (unsigned slice = 0; slice < numSlice + 1; ++slice)
		{
			float theta = slice * degreePerSlice;
			v.pos = glm::vec3(radius * cosf(phi) * cosf(theta), radius * sinf(phi), radius * cosf(phi) * sinf(theta));
			v.normal = glm::vec3(cosf(phi) * cosf(theta), sinf(phi), cosf(phi) * sinf(theta));
			vertex_buffer_data.push_back(v);
		}
	}

	for (unsigned stack = 0; stack < numStack; ++stack)
	{
		for (unsigned slice = 0; slice < numSlice + 1; ++slice)
		{
			index_buffer_data.push_back(hemisphereStart + (numSlice + 1) * stack + slice + 0);
			index_buffer_data.push_back(hemisphereStart + (numSlice + 1) * (stack + 1) + slice + 0);
		}
	}

	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLE_STRIP;

	return mesh;
}

Mesh* MeshBuilder::GenerateCylinder(const std::string& meshName, glm::vec3 color, int numSlice, float radius, float height)
{
	Vertex v;
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;

	v.color = color;

	float degreePerSlice = glm::two_pi<float>() / numSlice;

	// Bottom flat surface
	v.pos = glm::vec3(0.f, -0.5f * height, 0);
	v.normal = glm::vec3(0.f, -1.f, 0.f);
	v.texCoord = glm::vec2(0.5f, 0.5f);
	vertex_buffer_data.push_back(v);
	for (unsigned slice = 0; slice < numSlice + 1; ++slice)
	{
		float theta = slice * degreePerSlice;
		v.pos = glm::vec3(radius * cos(theta), -0.5f * height, radius * sin(theta));
		v.normal = glm::vec3(0.f, -1.f, 0.f);
		v.texCoord = glm::vec2(0.5f + 0.5f * cos(theta), 0.5f + 0.5f * sin(theta));
		vertex_buffer_data.push_back(v);
	}

	for (int slice = 0; slice < numSlice + 1; ++slice)
	{
		index_buffer_data.push_back(0);
		index_buffer_data.push_back(slice + 1);
	}

	unsigned middleStartIndex = vertex_buffer_data.size();
	for (int slice = 0; slice < numSlice + 1; ++slice)
	{
		float theta = slice * degreePerSlice;
		float u = (float)slice / numSlice;

		v.pos = glm::vec3(radius * cos(theta), -height * 0.5f, radius * sin(theta));
		v.normal = glm::vec3(glm::cos(theta), 0.f, glm::sin(theta));
		v.texCoord = glm::vec2(u, 0.f);

		vertex_buffer_data.push_back(v);

		v.pos = glm::vec3(radius * cos(theta), height * 0.5f, radius * sin(theta));
		v.normal = glm::vec3(cos(theta), 0.f, sin(theta));
		v.texCoord = glm::vec2(u, 1.f);
		vertex_buffer_data.push_back(v);
	}

	for (unsigned slice = 0; slice < numSlice + 1; slice++)
	{
		index_buffer_data.push_back(middleStartIndex + 2 * slice + 0);
		index_buffer_data.push_back(middleStartIndex + 1 + 2 * slice + 0);
	}

	// Top flat surface
	unsigned topStartIndex = vertex_buffer_data.size();
	v.pos = glm::vec3(0, 0.5f * height, 0);
	v.normal = glm::vec3(0.f, 1.f, 0.f);
	v.texCoord = glm::vec2(0.5f, 0.5f);
	vertex_buffer_data.push_back(v);

	for (unsigned slice = 0; slice < numSlice + 1; ++slice)
	{
		float theta = slice * degreePerSlice;
		v.pos = glm::vec3(radius * cos(theta), 0.5f * height, radius * sin(theta));
		v.normal = glm::vec3(0.f, 1.f, 0.f);
		v.texCoord = glm::vec2(0.5f + 0.5f * cos(theta), 0.5f + 0.5f * sin(theta));
		vertex_buffer_data.push_back(v);
	}

	for (int slice = 0; slice < numSlice + 1; ++slice)
	{
		index_buffer_data.push_back(topStartIndex + slice + 1);
		index_buffer_data.push_back(topStartIndex);
	}

	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLE_STRIP;

	return mesh;
}

Mesh* MeshBuilder::GenerateOBJ(const std::string& meshName, const std::string& file_path, int textureID)
{
	// Read vertices, texcoords & normals from OBJ
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool success = ModelLoader::LoadOBJ(file_path.c_str(), vertices, uvs, normals);

	if (!success) { return NULL; }

	// Index the vertices, texcoords & normals properly
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;
	ModelLoader::IndexVBO(vertices, uvs, normals, index_buffer_data, vertex_buffer_data);

	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLES;
	if (textureID != -1)
		mesh->textureID = textureID;

	return mesh;
}

Mesh* MeshBuilder::GenerateOBJMTL(const std::string& meshName, const std::string& file_path, const std::string& mtl_path, int textureID)
{
	//Read vertices, texcoords & normals from OBJ
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	std::vector<Material> materials;

	bool success = ModelLoader::LoadOBJMTL(file_path.c_str(), mtl_path.c_str(), vertices, uvs, normals, materials);
	if (!success) return NULL;

	//Index the vertices, texcoords & normals properly
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;
	ModelLoader::IndexVBO(vertices, uvs, normals, index_buffer_data, vertex_buffer_data);

	Mesh* mesh = new Mesh(meshName);
	for (Material& material : materials)
	{
		mesh->materials.push_back(material);
	}
		

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLES;
	if (textureID != -1)
		mesh->textureID = textureID;

	return mesh;
}

Mesh* MeshBuilder::GenerateText(const std::string& meshName, unsigned numRow, unsigned numCol, float advanceWidth, unsigned textureID) {

	Vertex v;
	std::vector<Vertex> vertex_buffer_data;
	std::vector<unsigned> index_buffer_data;

	float width = 1.f / numCol;
	float height = 1.f / numRow;
	unsigned offset = 0;
	v.normal = glm::vec3(0, 0, 1);

	for (unsigned row = 0; row < numRow; ++row)
	{
		for (unsigned col = 0; col < numCol; ++col)
		{

			v.pos = glm::vec3(0.5f * advanceWidth, 0.5f, 0.f);
			v.texCoord = glm::vec2(width * (col + advanceWidth), height * (numRow - row));
			vertex_buffer_data.push_back(v);

			v.pos = glm::vec3(-0.5f * advanceWidth, 0.5f, 0.f);
			v.texCoord = glm::vec2(width * (col + 0), height * (numRow - row));
			vertex_buffer_data.push_back(v);

			v.pos = glm::vec3(-0.5f * advanceWidth, -0.5f, 0.f);
			v.texCoord = glm::vec2(width * (col + 0), height * (numRow - 1 - row));
			vertex_buffer_data.push_back(v);

			v.pos = glm::vec3(0.5f * advanceWidth, -0.5f, 0.f);
			v.texCoord = glm::vec2(width * (col + advanceWidth), height * (numRow - 1 - row));
			vertex_buffer_data.push_back(v);

			index_buffer_data.push_back(0 + offset);
			index_buffer_data.push_back(1 + offset);
			index_buffer_data.push_back(2 + offset);
			index_buffer_data.push_back(0 + offset);
			index_buffer_data.push_back(2 + offset);
			index_buffer_data.push_back(3 + offset);

			offset += 4;

		}
	}

	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->mode = Mesh::DRAW_TRIANGLES;
	mesh->indexSize = index_buffer_data.size();
	mesh->textureID = textureID;

	return mesh;
}

Mesh* MeshBuilder::GenerateSkybox(const std::string& meshName, unsigned textureID) {
	Vertex v; // Vertex definition
	std::vector<Vertex> vertex_buffer_data; // Vertex Buffer Objects
	std::vector<GLuint> index_buffer_data; // Element Buffer Objects
	v.color = glm::vec3(1, 1, 1);
	float size = 1000;
	float width = size, height = size, length = size;

	unsigned reserveCount = 0;
	reserveCount = 4 * 6;
	vertex_buffer_data.reserve(reserveCount);

	// bottom
	v.normal = glm::vec3(0, 1, 0);
	v.texCoord = glm::vec2(0.25f, 0.125f);
	v.pos = glm::vec3(0.5f * width, -0.5f * height, -0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.25f, 0.375f);
	v.pos = glm::vec3(0.5f * width, -0.5f * height, 0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.5f, 0.375f);
	v.pos = glm::vec3(-0.5f * width, -0.5f * height, 0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.5f, 0.125f);
	v.pos = glm::vec3(-0.5f * width, -0.5f * height, -0.5f * length);
	vertex_buffer_data.push_back(v);

	// front
	v.normal = glm::vec3(0, 0, -1);
	v.texCoord = glm::vec2(0.25f, 0.375f);
	v.pos = glm::vec3(0.5f * width, -0.5f * height, 0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.25f, 0.625f);
	v.pos = glm::vec3(0.5f * width, 0.5f * height, 0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.5f, 0.625f);
	v.pos = glm::vec3(-0.5f * width, 0.5f * height, 0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.5f, 0.375f);
	v.pos = glm::vec3(-0.5f * width, -0.5f * height, 0.5f * length);
	vertex_buffer_data.push_back(v);

	// left
	v.normal = glm::vec3(-1, 0, 0);
	v.texCoord = glm::vec2(0, 0.375f);
	v.pos = glm::vec3(0.5f * width, -0.5f * height, -0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0, 0.625f);
	v.pos = glm::vec3(0.5f * width, 0.5f * height, -0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.25f, 0.625f);
	v.pos = glm::vec3(0.5f * width, 0.5f * height, 0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.25f, 0.375f);
	v.pos = glm::vec3(0.5f * width, -0.5f * height, 0.5f * length);
	vertex_buffer_data.push_back(v);

	// back
	v.normal = glm::vec3(0, 0, 1);
	v.texCoord = glm::vec2(0.75f, 0.375f);
	v.pos = glm::vec3(-0.5f * width, -0.5f * height, -0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.75f, 0.625f);
	v.pos = glm::vec3(-0.5f * width, 0.5f * height, -0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(1, 0.625f);
	v.pos = glm::vec3(0.5f * width, 0.5f * height, -0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(1, 0.375f);
	v.pos = glm::vec3(0.5f * width, -0.5f * height, -0.5f * length);
	vertex_buffer_data.push_back(v);

	// right
	v.normal = glm::vec3(1, 0, 0);
	v.texCoord = glm::vec2(0.5f, 0.375f);
	v.pos = glm::vec3(-0.5f * width, -0.5f * height, 0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.5f, 0.625f);
	v.pos = glm::vec3(-0.5f * width, 0.5f * height, 0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.75f, 0.625f);
	v.pos = glm::vec3(-0.5f * width, 0.5f * height, -0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.75f, 0.375f);
	v.pos = glm::vec3(-0.5f * width, -0.5f * height, -0.5f * length);
	vertex_buffer_data.push_back(v);

	// top
	v.normal = glm::vec3(0, -1, 0);
	v.texCoord = glm::vec2(0.25f, 0.625f);
	v.pos = glm::vec3(0.5f * width, 0.5f * height, 0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.25f, 0.875f);
	v.pos = glm::vec3(0.5f * width, 0.5f * height, -0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.5f, 0.875f);
	v.pos = glm::vec3(-0.5f * width, 0.5f * height, -0.5f * length);
	vertex_buffer_data.push_back(v);
	v.texCoord = glm::vec2(0.5f, 0.625f);
	v.pos = glm::vec3(-0.5f * width, 0.5f * height, 0.5f * length);
	vertex_buffer_data.push_back(v);

	reserveCount = 6 * 6;
	index_buffer_data.reserve(reserveCount);

	for (unsigned i = 0; i < 6; i++) {
		index_buffer_data.push_back(2 + i * 4);
		index_buffer_data.push_back(1 + i * 4);
		index_buffer_data.push_back(0 + i * 4);

		index_buffer_data.push_back(0 + i * 4);
		index_buffer_data.push_back(3 + i * 4);
		index_buffer_data.push_back(2 + i * 4);
	}

	// Create the new mesh
	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLES;
	mesh->textureID = textureID;

	return mesh;
}

Mesh* MeshBuilder::GenerateGround(const std::string& meshName, float size, float texSize, unsigned textureID)
{
	Vertex v;
	std::vector<Vertex> vertex_buffer_data;
	std::vector<GLuint> index_buffer_data;
	v.color = glm::vec3(1);
	v.normal = glm::vec3(0, 0, 1);
	float height = size, width = size;
	texSize = size / texSize;

	vertex_buffer_data.reserve(4);

	// Add the vertices here
	v.pos = glm::vec3(0.5f * width, -0.5f * height, 0.f);
	v.texCoord = glm::vec2(texSize, 0);
	vertex_buffer_data.push_back(v); //v0
	v.pos = glm::vec3(0.5f * width, 0.5f * height, 0.f);
	v.texCoord = glm::vec2(texSize, texSize);
	vertex_buffer_data.push_back(v); //v1
	v.pos = glm::vec3(-0.5f * width, 0.5f * height, 0.f);
	v.texCoord = glm::vec2(0, texSize);
	vertex_buffer_data.push_back(v); //v2
	v.pos = glm::vec3(-0.5f * width, -0.5f * height, 0.f);
	v.texCoord = glm::vec2(0, 0);
	vertex_buffer_data.push_back(v); //v3

	index_buffer_data.reserve(6);

	index_buffer_data.push_back(0);
	index_buffer_data.push_back(1);
	index_buffer_data.push_back(2);

	index_buffer_data.push_back(2);
	index_buffer_data.push_back(3);
	index_buffer_data.push_back(0);

	// Create the new mesh
	Mesh* mesh = new Mesh(meshName);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertex_buffer_data.size() * sizeof(Vertex), &vertex_buffer_data[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_data.size() * sizeof(GLuint), &index_buffer_data[0], GL_STATIC_DRAW);

	mesh->indexSize = index_buffer_data.size();
	mesh->mode = Mesh::DRAW_TRIANGLES;
	mesh->textureID = textureID;

	return mesh;
}
