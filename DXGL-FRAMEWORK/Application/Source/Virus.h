#ifndef VIRUS_H
#define VIRUS_H

#include <iostream>
#include <random>
#include "RenderObject.h"

class Virus
{
public:
	bool hasBeenDestroyed = false;
	bool hasBeenHit = false;
	bool returningToX = false;
	float homeX = 0.f;
	int hp = 10;
	float timerHit = 5.f;
	std::weak_ptr<RenderObject> virus;

	// Waypoint data members
	std::vector<glm::vec3> waypoints;
	size_t waypointIndex = 0;

	// Movement data members
	float moveSpeed = 3.f;
	float arriveRadius = 0.75f;
	float smoothing = 20.f;

	Virus(std::weak_ptr<RenderObject> virus) :
		virus(virus) {
	}

	std::vector<glm::vec3> BuildRandomPath(std::vector<glm::vec3>& portalPositionsList, std::mt19937& rng, std::uniform_int_distribution<int>& portalDistribution, int spawnIndex, bool hasSpawn);
};

#endif