#include "PhysicsManager.h"

void PhysicsManager::InitWorld() {
	world = physicsCommon.createPhysicsWorld();
}

void PhysicsManager::UpdatePhysics(double dt) {
	timeAccumulator += dt;

	// use this so that it always runs at constant step while keeping ralatively true to framerate
	while (timeAccumulator >= TIME_STEP) {
		world->update(TIME_STEP);
		timeAccumulator -= TIME_STEP;
	}
}
