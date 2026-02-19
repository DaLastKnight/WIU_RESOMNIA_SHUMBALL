#ifndef PLAYER_H
#define PLAYER_H

#include "RenderObject.h"

enum class GEOMETRY_TYPE : int;
class FPCamera;

class Player {
public:

	glm::vec3 position;
	glm::vec3 direction = glm::vec3(0, 0, 1);

	float speed = 3;
	glm::vec3 velocity;
	glm::vec3 smoothVelocity;
	float sprintMultiplier = 2;
	float smoothSprintMultiplier = 1;
	bool sprinting = false;

	std::weak_ptr<RenderObject> renderGroup;

	glm::vec3 cameraOffset = glm::vec3(0, 2, 0);

	bool allowControl = true;

	void Init(std::shared_ptr<RenderObject> parent, GEOMETRY_TYPE type, glm::vec3 cameraOffset);

	void UpdatePositionWithCamera(double dt, FPCamera& camera);
	void SyncRender();

	void VariableRefresh();

	Player();
	~Player();

private:


};

#endif