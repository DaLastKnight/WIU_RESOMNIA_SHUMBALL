#include "FPCamera.h"
#include "KeyboardController.h"
#include "MouseController.h"

//Include GLFW
#include <GLFW/glfw3.h>

using glm::vec3;


FPCamera::FPCamera() : phi(0.0f), theta(0.0f), psi(0), isDirty(false)
{
}

FPCamera::~FPCamera()
{
}

void FPCamera::Init(glm::vec3 position, glm::vec3 direction) {
	basePosition = position;
	smoothPosition = basePosition;

	SetDirection(direction);

	smoothTheta = theta;
	smoothPhi = phi;
	smoothPsi = psi;

	Refresh();
}

void FPCamera::Reset() {
	basePosition = smoothPosition = vec3();
	SetDirection(vec3(0, 0, -1));
}

void FPCamera::Update(double dt) {

	if (allowSelfMovement) {
		const float ROTATE_SPEED = 35.f;
		const float MOVE_SPEED = 5.f;
		const float FIRST_PERSON_SENSITIVITY = 0.5f;
		const float ROLL_SENSITIVITY = 0.2f;

		bool speedBoostActive = KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_LEFT_SHIFT);

		// camera rotation
		if (allowViewRotation) {
			// left right camera rotation
			if (MouseController::GetInstance()->GetMouseDeltaX() > 0) {

				float scaledSpeed = ROTATE_SPEED * MouseController::GetInstance()->GetMouseDeltaX();
				theta += scaledSpeed * FIRST_PERSON_SENSITIVITY * static_cast<float>(dt);
				psi += scaledSpeed * ROLL_SENSITIVITY * static_cast<float>(dt);
				isDirty = true;

			}
			else if (MouseController::GetInstance()->GetMouseDeltaX() < 0) {

				float scaledSpeed = ROTATE_SPEED * abs(MouseController::GetInstance()->GetMouseDeltaX());
				theta -= scaledSpeed * FIRST_PERSON_SENSITIVITY * static_cast<float>(dt);
				psi -= scaledSpeed * ROLL_SENSITIVITY * static_cast<float>(dt);
				isDirty = true;

			}

			// up down camera rotation
			if (MouseController::GetInstance()->GetMouseDeltaY() > 0) {

				float scaledSpeed = ROTATE_SPEED * MouseController::GetInstance()->GetMouseDeltaY();
				phi += scaledSpeed * FIRST_PERSON_SENSITIVITY * static_cast<float>(dt);
				isDirty = true;

			}
			else if (MouseController::GetInstance()->GetMouseDeltaY() < 0) {

				float scaledSpeed = ROTATE_SPEED * abs(MouseController::GetInstance()->GetMouseDeltaY());
				phi -= scaledSpeed * FIRST_PERSON_SENSITIVITY * static_cast<float>(dt);
				isDirty = true;
			}
		}

		// camera movements
		if (allowPositionMovement) {
			vec3 moveDire = finalTarget - finalPosition;
			moveDire.y = 0;
			moveDire = glm::normalize(moveDire);
			vec3 lr = glm::normalize(glm::cross(moveDire, vec3(0, 1, 0))); // left right vector
			vec3 ud = glm::normalize(glm::cross(moveDire, lr)); // up down vector 

			// left right movement
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_D)) {

				float boostedSpeed = MOVE_SPEED;
				if (speedBoostActive)
					boostedSpeed *= 2.f;
				basePosition += lr * boostedSpeed * static_cast<float>(dt);

				isDirty = true;
			}
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_A)) {

				float boostedSpeed = MOVE_SPEED;
				if (speedBoostActive)
					boostedSpeed *= 2.f;
				basePosition -= lr * boostedSpeed * static_cast<float>(dt);

				isDirty = true;
			}

			// forward back movement
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_W)) {

				float boostedSpeed = MOVE_SPEED;
				if (speedBoostActive)
					boostedSpeed *= 2.f;
				basePosition += moveDire * boostedSpeed * static_cast<float>(dt);

				isDirty = true;
			}
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_S)) {

				float boostedSpeed = MOVE_SPEED;
				if (speedBoostActive)
					boostedSpeed *= 2.f;
				basePosition -= moveDire * boostedSpeed * static_cast<float>(dt);

				isDirty = true;
			}

			// up down movement
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_Q)) {

				float boostedSpeed = MOVE_SPEED;
				if (speedBoostActive)
					boostedSpeed *= 2.f;
				basePosition += ud * boostedSpeed * static_cast<float>(dt);

				isDirty = true;
			}
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_E)) {

				float boostedSpeed = MOVE_SPEED;
				if (speedBoostActive)
					boostedSpeed *= 2.f;
				basePosition -= ud * boostedSpeed * static_cast<float>(dt);

				isDirty = true;
			}

			// return basePosition to origin
			if (KeyboardController::GetInstance()->IsKeyDown(GLFW_KEY_0)) {
				basePosition = glm::vec3(0, 0, 0);
				isDirty = true;
			}
		}
	}
	else {
		isDirty = true;
	}

	if (smoothTheta != theta || smoothPhi != phi || smoothPsi != psi || smoothPosition != basePosition)
		isDirty = true;

	if (!isDirty) return;

	// conditions
	if (phi > 89)
		phi = 89;
	if (phi < -89)
		phi = -89;

	// Calculate smoothed stats
	smoothPhi += (phi - smoothPhi) / rotSmoothing * dt * 100;
	smoothTheta += (theta - smoothTheta) / rotSmoothing * dt * 100;
	smoothPsi += (psi - smoothPsi) / moveSmoothing * dt * 100;
	vec3 posDiff = basePosition - smoothPosition;
	smoothPosition += (posDiff / moveSmoothing) * static_cast<float>(dt) * 100.f;

	// bobbing
	bobbingElapsed += dt * bobbingSmoothSpeed;
	if (bobbingActive) {
		float targetX = sinf(5 * bobbingElapsed) * bobbingMaxX;
		float targetY = sinf(10 * bobbingElapsed) * bobbingMaxY;
		float targetPsi = sinf(5 * bobbingElapsed) * bobbingMaxPsi;
		bobbingX += (targetX - bobbingX) / moveSmoothing * dt * 100;
		bobbingY += (targetY - bobbingY) / moveSmoothing * dt * 100;
		bobbingPsi += (targetPsi - bobbingPsi) / moveSmoothing * dt * 100;
	}
	else {
		bobbingX += (-bobbingX) / moveSmoothing * dt * 100;
		bobbingY += (-bobbingY) / moveSmoothing * dt * 100;
		bobbingPsi += (-bobbingPsi) / moveSmoothing * dt * 100;
	}

	Refresh();
}

void FPCamera::ForceNextRefresh() {
	isDirty = true;
}

void FPCamera::ForceRefresh() {
	Refresh();
}

void FPCamera::SetDirection(glm::vec3 direction, float rollAngleDeg) {
	direction = glm::normalize(direction);
	phi = glm::degrees(asinf(direction.y));
	theta = glm::degrees(atan2f(direction.z, direction.x));
	psi = rollAngleDeg;
}

void FPCamera::VariableRefresh() {
	psi = 0;
	isDirty = false;
	bobbingActive = false;
}

void FPCamera::Set(MODE mode) {
	if (mode == MODE::PAUSE)
		MouseController::GetInstance()->setMouseEnabled(true);
	else 
		MouseController::GetInstance()->setMouseEnabled(false);

	switch (mode) {
	case MODE::FREE:
		allowSelfMovement = true;
		allowPositionMovement = true;
		break;
	case MODE::FIRST_PERSON:
		allowSelfMovement = true;
		allowPositionMovement = false;
		break;
	case MODE::PAUSE:
	case MODE::LOCK_ON:
		allowSelfMovement = false;
		break;
	default:
		break;
	}

	currentMode = mode;
}

void FPCamera::Refresh()
{
	// find smoothPosition's target
	float radPhi = glm::radians(smoothPhi);
	float radTheta = glm::radians(smoothTheta);

	float x = smoothPosition.x + cosf(radPhi) * cosf(radTheta);
	float y = smoothPosition.y + sinf(radPhi);
	float z = smoothPosition.z + cosf(radPhi) * sinf(radTheta);
	smoothTarget = glm::vec3(x, y, z);

	// find the direction of current view
	direction = glm::normalize(smoothTarget - smoothPosition);
	// Find the right vector using default up (0,1,0) first
	glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0.f, 1.f, 0.f))); // the up here should be opposite direction of gravity
	// calculate the up vector without roll
	up = glm::normalize(glm::cross(right, direction));

	// calculate final stats
	float radPsi = glm::radians(smoothPsi + bobbingPsi);
	finalUp = up * cosf(radPsi) + right * sinf(radPsi); // adding roll to up

	vec3 bobbing = finalUp * bobbingY + right * bobbingX;

	finalPosition = smoothPosition + bobbing;
	finalTarget = smoothTarget + bobbing;

	finalDirection = glm::normalize(finalTarget - finalPosition);
	if (finalDirection.y != 0) {
		// ViewIntersectXZPlane = usedPos + distance(view)
		// to find intersect on xz plane, the y value of ViewIntersectXZPlane = 0;
		// y equaltion: 0 = usedPos.y + diance(view.y)
		// change of subject to find distance v
		float distance = (0 - finalPosition.y) / finalDirection.y;
		ViewIntersectXZPlane = finalPosition + distance * finalDirection;
	}
}
