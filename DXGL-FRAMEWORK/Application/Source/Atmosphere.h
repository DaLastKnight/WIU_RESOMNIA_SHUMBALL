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

	Atmosphere(glm::vec3 color = glm::vec3(0.05f, 0.07f, 0.1f), float fogDensity = 1, float fogVisibility = 0.000001f, float lightestRange = 2, float densestRange = 20)
		: color(color), fogDensity(fogDensity), fogVisibility(fogVisibility), lightestRange(lightestRange), densestRange(densestRange) {};

	void Set(glm::vec3 color, float fogDensity, float fogVisibility, float lightestRange, float densestRange) {
		this->color = color;
		this->fogDensity = fogDensity;
		this->fogVisibility = fogVisibility;
		this->lightestRange = lightestRange;
		this->densestRange = densestRange;
	}

	Atmosphere operator+(const Atmosphere& other) {
		return Atmosphere(this->color + other.color, this->fogDensity + other.fogDensity, this->fogVisibility + other.fogVisibility, this->lightestRange + other.lightestRange, this->densestRange + other.densestRange);
	}
	Atmosphere operator-(const Atmosphere& other) {
		return Atmosphere(this->color - other.color, this->fogDensity - other.fogDensity, this->fogVisibility - other.fogVisibility, this->lightestRange - other.lightestRange, this->densestRange - other.densestRange);
	}
	Atmosphere operator*(float scalar) {
		return Atmosphere(this->color * scalar, this->fogDensity * scalar, this->fogVisibility * scalar, this->lightestRange * scalar, this->densestRange * scalar);
	}
	Atmosphere operator/(float scalar) {
		return Atmosphere(this->color / scalar, this->fogDensity / scalar, this->fogVisibility / scalar, this->lightestRange / scalar, this->densestRange / scalar);
	}
};

#endif
