#define _USE_MATH_DEFINES
#include <cmath>

#include "Player.h"
#include "FPCamera.h"
#include "PhysicsManager.h"

#include "Utils.h"

#include "Console.h"

using glm::vec3;

void Player::Init(std::shared_ptr<RenderObject> parent, int type, glm::vec3 cameraOffset) {
	parent->NewChild(MeshObject::Create(type));
	this->cameraOffset = cameraOffset;

	auto& newObj = RenderObject::newObject;
	renderGroup = newObj;
	newObj->name = "player";
	newObj->AddPhysics(PhysicsObject::DYNAMIC);

	physics = newObj->GetPhysics();
	physics->AddCollider(PhysicsObject::CAPSULE, vec3(0.5f, 1, 0));
	physics->SetMaterial(0, 0.5f, 0.5f);
	physics->UpdateMassProperties();
	physics->SetPosition(vec3(0, 10, 0));
	physics->SetAllowedRotationAxes(vec3(0, 1, 0));
	physics->SetAllowSleep(false);

	newObj.reset();
}

void Player::UpdatePhysicsWithCamera(double dt, FPCamera& camera) {
	direction = camera.GetPlainDirection();
	direction.y = 0;
	direction = glm::normalize(direction);

	float targetSprintMultiplier = 1;
	float targetBobbingSpeed = 0;

	if (velocity != vec3(0)) {
		int targetBobbingMultiplier = (glm::dot(velocity, direction) >= 0 ? 1 : -1);

		velocity *= speed * dt;

		camera.bobbingActive = true;
		targetBobbingSpeed = camera.bobbingSpeed;

		vec3 finalTrlDire = glm::normalize(velocity);
		// if actually sprinting and moving forward
		if (sprinting = (sprinting && glm::dot(velocity, direction) > 0)) {
			targetSprintMultiplier = sprintMultiplier;
			camera.psi *= 1 + 1 * (smoothSprintMultiplier / sprintMultiplier) * dt * 100 * camera.sprintPsiMultiplier;
			targetBobbingSpeed *= 1 + 1 * (smoothSprintMultiplier / sprintMultiplier) * dt * 100 * targetBobbingMultiplier;
		}

		smoothSprintMultiplier = Smooth(smoothSprintMultiplier, targetSprintMultiplier, 20.f, dt);
		velocity = finalTrlDire * speed * smoothSprintMultiplier * static_cast<float>(dt);

		smoothVelocity = Smooth(smoothVelocity, velocity, 7.5f, dt);
		physics->SetVelocity(smoothVelocity + physics->GetVelocity() * vec3(0, 1, 0));

		camera.basePosition = position + cameraOffset;
		camera.basePosition += smoothVelocity * camera.movePositionOffset - smoothVelocity;
	}
	else {
		smoothSprintMultiplier = Smooth(smoothSprintMultiplier, targetSprintMultiplier, 20.f, dt);

		smoothVelocity = Smooth(smoothVelocity, velocity, 7.5f, dt);
		physics->SetVelocity(smoothVelocity + physics->GetVelocity() * vec3(0, 1, 0));

		camera.basePosition = position + cameraOffset;
	}

	camera.bobbingSmoothSpeed = Smooth(camera.bobbingSmoothSpeed, targetBobbingSpeed, 20.f, dt);
}

void Player::UpdatePhysics(double dt) {
	float targetSprintMultiplier = 1;
	float targetBobbingSpeed = 0;

	if (velocity != vec3(0)) {

		vec3 finalTrlDire = glm::normalize(velocity);
		// if actually sprinting and moving forward
		if (sprinting = (sprinting && glm::dot(finalTrlDire, direction) > 0)) {
			targetSprintMultiplier = sprintMultiplier;
		}

		smoothSprintMultiplier = Smooth(smoothSprintMultiplier, targetSprintMultiplier, 20.f, dt);
		velocity = finalTrlDire * speed * smoothSprintMultiplier * static_cast<float>(dt);

		smoothVelocity = Smooth(smoothVelocity, velocity, 7.5f, dt);
		physics->SetVelocity(smoothVelocity + physics->GetVelocity() * vec3(0, 1, 0));
	}
	else {
		smoothVelocity = Smooth(smoothVelocity, velocity, 7.5f, dt);
		physics->SetVelocity(smoothVelocity + physics->GetVelocity() * vec3(0, 1, 0));
	}
}

void Player::SyncPhysics() {
	auto obj = renderGroup.lock();
	position = physics->GetPostion();
	obj->trl = position;
	obj->rot.y = atan2f(direction.z, direction.x);
	obj->UpdateModel();
}

void Player::VariableRefresh() {
	velocity = vec3(0);
}

Player::Player() {}

Player::~Player() {}
