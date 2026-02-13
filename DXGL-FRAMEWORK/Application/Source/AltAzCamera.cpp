#include "AltAzCamera.h"
#include "KeyboardController.h"
#include "MouseController.h"

//Include GLFW
#include <GLFW/glfw3.h>

AltAzCamera::AltAzCamera() : phi(0.0f), theta(0.0f), distance(0.0f), isDirty(false), right(0.f, 0.f, 0.f)
{
	position = glm::vec3(0.f, 2.f, 6.f);
}

AltAzCamera::~AltAzCamera()
{
}

void AltAzCamera::Init(float theta, float phi, float distance)
{
	this->theta = theta;
	this->phi = phi;
	this->distance = distance;
	this->isDirty = true;

	Refresh();
}

void AltAzCamera::Reset()
{
}

void AltAzCamera::Update(double dt)
{
	static const float ROTATE_SPEED = 10.0f;
	static const float ZOOM_SPEED = 20.0f;

	if (MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_YOFFSET) > 0) {
		distance -= ZOOM_SPEED * static_cast<float>(dt);
		isDirty = true;
	}
	else if (MouseController::GetInstance()->GetMouseScrollStatus(MouseController::SCROLL_TYPE_YOFFSET) < 0) {
		distance += ZOOM_SPEED * static_cast<float>(dt);
		isDirty = true;
	}

	// For mouse controls
	if (MouseController::GetInstance()->IsButtonPressed(MouseController::RMB) && MouseController::GetInstance()->GetMouseDeltaX() > 0) {
		theta -= MouseController::GetInstance()->GetMouseDeltaX() * ROTATE_SPEED * static_cast<float>(dt);
		isDirty = true;
	}

	if (MouseController::GetInstance()->IsButtonPressed(MouseController::RMB) && MouseController::GetInstance()->GetMouseDeltaX() < 0) {
		theta += -MouseController::GetInstance()->GetMouseDeltaX() * ROTATE_SPEED * static_cast<float>(dt);
		isDirty = true;
	}

	if (MouseController::GetInstance()->IsButtonPressed(MouseController::RMB) && MouseController::GetInstance()->GetMouseDeltaY() > 0) {
		phi += MouseController::GetInstance()->GetMouseDeltaY() * ROTATE_SPEED * static_cast<float>(dt);
		isDirty = true;
	}

	if (MouseController::GetInstance()->IsButtonPressed(MouseController::RMB) && MouseController::GetInstance()->GetMouseDeltaY() < 0) {
		phi -= -MouseController::GetInstance()->GetMouseDeltaY() * ROTATE_SPEED * static_cast<float>(dt);
		isDirty = true;
	}

	this->Refresh();
}

void AltAzCamera::Refresh()
{
	if (!isDirty) return;

	

	// Calculate the position based on distance
	float x = this->distance * cosf(glm::radians(this->phi)) * cosf(glm::radians(this->theta));
	float y = this->distance * sinf(glm::radians(this->phi));
	float z = this->distance * cosf(glm::radians(this->phi)) * sinf(glm::radians(this->theta));
	this->position = glm::vec3(x, y, z);

	// Set default target to origin for this camera
	this->target = glm::vec3(0.f);
	glm::vec3 view = glm::normalize(target - position);

	// Find the right vector using default up (0,1,0) first
	right = glm::normalize(glm::cross(view, glm::vec3(0.f, 1.f, 0.f)));

	// Recalculate the up vector
	this->up = glm::normalize(glm::cross(right, view));

	isDirty = false;
}