#ifndef ICA2_STRUCT_H
#define ICA2_STRUCT_H

#include "GL\glew.h"

// GLM Headers
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_inverse.hpp>

//Include GLFW
#include <GLFW/glfw3.h>

#include "KeyboardController.h"

struct AABB
{
	// Uses x and z
	glm::vec2 min{};
	glm::vec2 max{};

	// Creates a temporary AABB for checking collision
	static AABB InitAABB(const glm::vec3& pos, const glm::vec2& halfExtent)
	{
		
		glm::vec2 temp{ pos.x, pos.z };
		return { temp - halfExtent, temp + halfExtent };
	}

	// Check collision
	bool CollideAABB(const AABB& box1, const AABB& box2)
	{
		if (box1.max.x < box2.min.x) return false;
		if (box1.min.x > box2.max.x) return false;
		if (box1.max.y < box2.min.y) return false;
		if (box1.min.y > box2.max.y) return false;

		return true;
		// y is z in this case
	}
};

struct Interactable
{
	// Checks for interaction using circle-to-circle collision check
	// as well as a boolean flag for showing UI or markers
	glm::vec3 pos{};
	float interactRadius = 0.f;
	bool canInteract = false;

	bool CheckInteractionDist(const glm::vec3& pos2)
	{
		// Check whether interaction is in range
		glm::vec3 diff = pos - pos2;
		diff.y = 0.f;

		float distSQ = glm::dot(diff, diff);
		float radiusSQ = interactRadius * interactRadius;
		return (distSQ <= radiusSQ);
	}

	bool TriggerInteraction()
	{
		return (KeyboardController::GetInstance()->IsKeyPressed(GLFW_KEY_E)); // E key
	}
};

struct Wall
{
	// Not just actual walls, but mainly an invisible barrier
	AABB box{};
	glm::vec3 pos{};
	glm::vec2 halfExtent{}; // x and z axes only

	static Wall InitWall(glm::vec3 wallPos, glm::vec2 wallHalfExtent)
	{
		Wall newWall;
		newWall.pos = wallPos;
		newWall.halfExtent = wallHalfExtent;
		newWall.box = AABB::InitAABB(wallPos, wallHalfExtent);
		return newWall;
	}
};

struct Crate
{
	// Special AABB in the scene that can get pushed around
	AABB box{};
	glm::vec3 pos{};
	glm::vec2 halfExtent{}; // x and z axes only
	int crateNum = 0;
	float mass = 1.f;
};

struct Skeleton
{
	// An interactive NPC with dialogue!
	AABB box{};
	glm::vec3 pos{};
	glm::vec2 halfExtent{}; // x and z axes only
	Interactable interaction;
	int timesInteracted = 0;
};

struct Player
{
	// Movement control and collider
	glm::vec3 pos{};
	glm::vec3 forward{};
	glm::vec3 right{};
	glm::vec2 halfExtent{ 1.f, 1.f };
	AABB box;

	// Variables for view bobbing
	float bobTimer = 0.f;
	float bobAmount = 0.f;
	float bobOffsetY = 0.f;

	float bobFreq = 0.8f;
	float bobAmp = 0.1f;
	float bobSmooth = 12.f;

	// Some extra movement variables
	float speed = 10.f;
	float mass = 1.f;
	bool isMoving = false;
};

struct Monk
{
	// AABB that just moves towards you
	glm::vec3 pos;
	glm::vec2 halfExtent{ 1.f, 1.f };
	AABB box;
	float mass = 1.f;
};

struct Fog
{
	// color is the fog color that takes up the screen
	// density is how much fog that takes up the screen (0.f - 1.f)
	// minDist is the distance from the player when the fog kicks in
	// maxDist is the distance from the player when the fog reaches maximum fogDensity
	glm::vec3 color{ 0.01f, 0.01f, 0.01f };
	float density = 0.9f;
	float minDist = 25.f;
	float maxDist = 80.f;
};

#endif