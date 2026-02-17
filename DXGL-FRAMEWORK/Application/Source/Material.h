#ifndef MATERIAL_H
#define MATERIAL_H

// GLM Headers
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

struct Material {

	enum MATERIAL_TYPE {

		MATT,
		POLISHED_METAL,
		ROUGH_METAL,
		SOFT,
		SEMI_GLOSS,
		GLOSS,
		TRANSPARENT_SMOOTH,
		ROUGH_ORGANIC,
		LACQUERED,
		WET,
		NEON,
		BRIGHT,
		NO_LIGHT,

		CUSTOM,
		MESH_MATERIAL,

		TOTAL_TYPE,
	};

	glm::vec3 kAmbient;
	glm::vec3 kDiffuse;
	glm::vec3 kSpecular;
	float kShininess;
	unsigned size;

	MATERIAL_TYPE type = CUSTOM;

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

	void Set(MATERIAL_TYPE type) {
		this->type = type;
		switch (type) {
		case MATT:
			kAmbient = glm::vec3(0.2);
			kDiffuse = glm::vec3(0.8);
			kSpecular = glm::vec3(0.05);
			kShininess = 8;
			break;

		case POLISHED_METAL:
			kAmbient = glm::vec3(0.15);
			kDiffuse = glm::vec3(0.9);
			kSpecular = glm::vec3(1);
			kShininess = 128;
			break;

		case ROUGH_METAL:
			kAmbient = glm::vec3(0.1);
			kDiffuse = glm::vec3(0.3);
			kSpecular = glm::vec3(0.6);
			kShininess = 48;
			break;

		case SOFT:
			kAmbient = glm::vec3(0.05);
			kDiffuse = glm::vec3(0.9);
			kSpecular = glm::vec3(0.02);
			kShininess = 4;
			break;

		case SEMI_GLOSS:
			kAmbient = glm::vec3(0.1);
			kDiffuse = glm::vec3(0.7);
			kSpecular = glm::vec3(0.3);
			kShininess = 32;
			break;

		case GLOSS:
			kAmbient = glm::vec3(0.1);
			kDiffuse = glm::vec3(0.6);
			kSpecular = glm::vec3(0.8);
			kShininess = 64;
			break;

		case TRANSPARENT_SMOOTH:
			kAmbient = glm::vec3(0.05);
			kDiffuse = glm::vec3(0.1);
			kSpecular = glm::vec3(0.9);
			kShininess = 128;
			break;

		case ROUGH_ORGANIC:
			kAmbient = glm::vec3(0.25);
			kDiffuse = glm::vec3(0.75);
			kSpecular = glm::vec3(0.05);
			kShininess = 6;
			break;

		case LACQUERED:
			kAmbient = glm::vec3(0.25);
			kDiffuse = glm::vec3(0.6);
			kSpecular = glm::vec3(0.3);
			kShininess = 32;
			break;

		case WET:
			kAmbient = glm::vec3(0.1);
			kDiffuse = glm::vec3(0.5);
			kSpecular = glm::vec3(0.7);
			kShininess = 64;
			break;

		case NEON:
			kAmbient = glm::vec3(1);
			kDiffuse = glm::vec3(0.1);
			kSpecular = glm::vec3(0.1);
			kShininess = 1;
			break;

		case BRIGHT:
			kAmbient = glm::vec3(1);
			kDiffuse = glm::vec3(0);
			kSpecular = glm::vec3(0);
			kShininess = 1;
			break;

		default:
			break;
		}
	}
};

#endif
