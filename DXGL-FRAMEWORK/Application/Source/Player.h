#ifndef PLAYER_H
#define PLAYER_H

#include "RenderObject.h"

class FPCamera;

class Player {
public:

	glm::vec3 position;
	glm::vec3 direction = glm::vec3(0, 0, 1);

	float speed = 200;
	glm::vec3 velocity;
	glm::vec3 smoothVelocity;
	float sprintMultiplier = 2;
	float smoothSprintMultiplier = 1;
	bool sprinting = false;

	std::weak_ptr<RenderObject> renderGroup;

	glm::vec3 cameraOffset = glm::vec3(0, 2, 0);

	bool allowControl = true;

	void Init(std::shared_ptr<RenderObject> parent, int type, glm::vec3 cameraOffset);

	void UpdatePhysicsWithCamera(double dt, FPCamera& camera);
	void UpdatePhysics(double dt);
	void SyncPhysics();

	void VariableRefresh();

	Player();
	~Player();

private:
	PhysicsObject* physics;

};

#endif