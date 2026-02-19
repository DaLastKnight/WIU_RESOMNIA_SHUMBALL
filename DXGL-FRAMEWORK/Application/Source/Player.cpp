#define _USE_MATH_DEFINES
#include <cmath>

#include "Player.h"
#include "FPCamera.h"

#include "Utils.h"

#include "Console.h"

using glm::vec3;

void Player::Init(std::shared_ptr<RenderObject> parent, int type, glm::vec3 cameraOffset) {
	parent->NewChild(MeshObject::Create(type));
	renderGroup = RenderObject::newObject;

	RenderObject::newObject.reset();

	this->cameraOffset = cameraOffset;
}

void Player::UpdatePositionWithCamera(double dt, FPCamera& camera) {
	direction = camera.GetFinalDirection();
	direction.y = 0;

	float targetSprintMultiplier = 1;
	float targetBobbingSpeed = 0;

	if (velocity != vec3(0)) {

		camera.bobbingActive = true;
		targetBobbingSpeed = camera.bobbingSpeed;

		vec3 finalTrlDire = glm::normalize(velocity);
		// if actually sprinting and moving forward
		if (sprinting && glm::dot(finalTrlDire, direction) > 0) {
			targetSprintMultiplier = sprintMultiplier;
			camera.psi *= 1 + 1 * (smoothSprintMultiplier / sprintMultiplier) * dt * 100;
			targetBobbingSpeed *= 1 + 1 * (smoothSprintMultiplier / sprintMultiplier) * dt * 100;
		}

		smoothSprintMultiplier = Smooth(smoothSprintMultiplier, targetSprintMultiplier, 20.f, dt);
		velocity = finalTrlDire * speed * smoothSprintMultiplier * static_cast<float>(dt);

		smoothVelocity = Smooth(smoothVelocity, velocity, 7.5f, dt);
		position += smoothVelocity;

		camera.basePosition = position + cameraOffset;
		camera.basePosition += smoothVelocity * camera.movePositionOffset * smoothSprintMultiplier - smoothVelocity;
	}
	else {
		smoothVelocity = Smooth(smoothVelocity, velocity, 7.5f, dt);
		position += smoothVelocity;

		camera.basePosition = position + cameraOffset;
	}

	camera.bobbingSmoothSpeed = Smooth(camera.bobbingSmoothSpeed, targetBobbingSpeed, 20.f, dt);
}

void Player::SyncRender() {
	auto obj = renderGroup.lock();
	obj->trl = position;
	obj->rot.y = atan2f(direction.z, direction.x);
}

void Player::VariableRefresh() {
	velocity = vec3(0);
}

Player::Player() {}

Player::~Player() {}
