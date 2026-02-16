#ifndef MATERIAL_H
#define MATERIAL_H

// GLM Headers
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

struct Material
{
	glm::vec3 kAmbient;
	glm::vec3 kDiffuse;
	glm::vec3 kSpecular;
	float kShininess;
	unsigned size;

	Material() : kAmbient(0.1f), kDiffuse(0.8f), kSpecular(0.5f), kShininess(4.f), size(0) {}

	Material& operator=(const Material& rhs)
	{
		kAmbient = rhs.kAmbient;
		kDiffuse = rhs.kDiffuse;
		kSpecular = rhs.kSpecular;
		kShininess = rhs.kShininess;
		size = rhs.size;
		return *this;
	}

	void SetZero() {
		Set(glm::vec3(0), glm::vec3(0), glm::vec3(0), 0);
	}

	void Set(glm::vec3 kAmbient, glm::vec3 kDiffuse, glm::vec3 kSpecular, float kShininess) {
		this->kAmbient = kAmbient;
		this->kDiffuse = kDiffuse;
		this->kSpecular = kSpecular;
		this->kShininess = kShininess;
	}
};

#endif
