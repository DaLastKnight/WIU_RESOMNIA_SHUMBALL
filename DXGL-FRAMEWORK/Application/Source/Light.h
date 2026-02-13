#ifndef LIGHT_H
#define LIGHT_H

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

struct Light
{
	enum LIGHT_TYPE
	{
		LIGHT_POINT = 0,
		LIGHT_DIRECTIONAL,
		LIGHT_SPOT,
		TOTAL_LIGHT_TYPE
	};
	
	LIGHT_TYPE type;

	glm::vec3 position;
	glm::vec3 color;
	float power;
	float kC, kL, kQ;

	glm::vec3 spotDirection;
	float cosCutoff;
	float cosInner;
	float exponent;

	Light() : position(0.f, 20.f, 0.f), color(1.f, 1.f, 1.f), power(1.f), kC(1.f), kL(0.1f), kQ(0.001f), type(LIGHT_POINT), spotDirection(1.f), cosCutoff(45.f), cosInner(30.f), exponent(1.f) {}
};

#endif
