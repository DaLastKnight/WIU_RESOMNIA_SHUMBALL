#ifndef ATMOSPHERE_H
#define ATMOSPHERE_H

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

struct Atmosphere {
	glm::vec3 color;
	float fogDensity;
	float fogVisibility;
	float lightestRange;
	float densestRange;

	Atmosphere()
		: color(glm::vec3(0.05f, 0.07f, 0.1f)), fogDensity(1), fogVisibility(0.000001f), lightestRange(2), densestRange(20) {}

	void Set(glm::vec3 color, float fogDensity, float fogVisibility, float lightestRange, float densestRange) {
		this->color = color;
		this->fogDensity = fogDensity;
		this->fogVisibility = fogVisibility;
		this->lightestRange = lightestRange;
		this->densestRange = densestRange;
	}
};

#endif
