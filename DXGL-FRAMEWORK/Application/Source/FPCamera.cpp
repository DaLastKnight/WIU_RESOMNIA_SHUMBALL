#include <iostream>
#include "FPCamera.h"
#include "KeyboardController.h"
#include "MouseController.h"

//Include GLFW
#include <GLFW/glfw3.h>

FPCamera::FPCamera() : isDirty(false)
{
	this->position = glm::vec3(0, 0, 0);
	this->right = glm::vec3(0, 0, 0);
	this->front = glm::vec3(0, 0, 0);
	this->up = glm::vec3(0, 1, 0);
	this->target = glm::vec3(0, 0, 0);
	yaw = 0.f;
	pitch = 0.f;
	lockSmooth = 12.f;
	isLockedOn = false;
	shouldCapture = true;
}

FPCamera::~FPCamera()
{
}

void FPCamera::Init(glm::vec3 position, glm::vec3 up)
{
	this->position = position;
	this->up = up;
	this->isDirty = true;

	Refresh();
}

void FPCamera::Reset()
{
}

void FPCamera::Update(double dt)
{
	static const float ROTATE_SPEED = 5.f;
	
	if (!isLockedOn && shouldCapture)
	{
		// Retrieves how much mousePos has changed since last Update()
		double deltaX = MouseController::GetInstance()->GetMouseDeltaX();
		double deltaY = MouseController::GetInstance()->GetMouseDeltaY();

		float angleHori = deltaX * ROTATE_SPEED * 4 * static_cast<float>(dt);
		float angleVert = deltaY * ROTATE_SPEED * 4 * static_cast<float>(dt);

		// Clamp pitch to prevent weird camera behaviour
		yaw += angleHori;
		pitch += angleVert;
		pitch = glm::clamp(pitch, -89.f, 89.f);

		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)); // Scales the rotation around y-axis
		front.y = sin(glm::radians(pitch)); // Direct vertical height (no yaw)
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch)); // Scales the rotation around y-axis
		front = glm::normalize(front);
	}
	else if (isLockedOn)
	{
		glm::vec3 desired = target - position;

		// Prevent lockedOn if target.pos and camera.pos share almost the exact same position
		if (glm::dot(desired, desired) > 0.000001f)
		{
			desired = glm::normalize(desired);

			// Smooth transition to locked/unlocked camera mode
			float t = 1.f - expf(-lockSmooth * static_cast<float>(dt));
			front = glm::normalize(glm::mix(front, desired, t));

			pitch = glm::degrees(asinf(front.y));
			yaw = glm::degrees(atan2f(front.z, front.x));
		}
	}
	else
	{
		MouseController::GetInstance()->GetMouseDeltaX();
		MouseController::GetInstance()->GetMouseDeltaY();
	}

	right = glm::normalize(glm::cross(front, glm::vec3(0, 1, 0)));
	up = glm::normalize(glm::cross(right, front));

	isDirty = true;

	this->Refresh();
}

void FPCamera::Refresh()
{
	if (!isDirty) return;

	isDirty = false;
}