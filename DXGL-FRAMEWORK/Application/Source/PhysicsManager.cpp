#include "PhysicsManager.h"

#include <glm/gtc/type_ptr.hpp>
#include "Console.h"

using namespace reactphysics3d;



/********************************* PhysicsObject *********************************/

void PhysicsObject::AddImpulse(glm::vec3 force) {
	body->setLinearVelocity(rp3d::Vector3(0, 0, 0));
	body->applyLocalForceAtCenterOfMass(Vec3Convert(force) * PhysicsManager::GetInstance().Get_TIME_STEP());
}
void PhysicsObject::AddSoftImpulse(glm::vec3 impulse) {
	body->applyLocalForceAtCenterOfMass(Vec3Convert(impulse) * PhysicsManager::GetInstance().Get_TIME_STEP());
}

void PhysicsObject::SetTransform(glm::vec3 position_vec3, glm::vec3 eulerRotation) {
	body->setTransform(Vec3ToRp3dTransform(position_vec3, eulerRotation));
}

void PhysicsObject::SetPosition(glm::vec3 position_vec3) {
	body->setTransform(Transform(Vec3Convert(position_vec3), body->getTransform().getOrientation()));
}

void PhysicsObject::SetOrientation(glm::vec3 eulerRotation) {
	body->setTransform(Transform(body->getTransform().getPosition(), EulerToQuaternion(eulerRotation)));
}

void PhysicsObject::InterpolateTransform() {
	Transform newTransform = body->getTransform();
	double factor = PhysicsManager::GetInstance().GetTimeAccumulator() / PhysicsManager::GetInstance().Get_TIME_STEP();
	Transform interpolatedTransform = Transform::interpolateTransforms(currentTransform, newTransform, factor);
	currentTransform = newTransform;
}

glm::mat4 PhysicsObject::GetModel() {
	float matrix[16];
	body->getTransform().getOpenGLMatrix(matrix);
	return glm::make_mat4(matrix);
}

glm::vec3 PhysicsObject::GetPosition() {
	return Vec3Convert(body->getTransform().getPosition());
}

glm::quat PhysicsObject::GetOrientation() {
	return  QuaternionToQuat(body->getTransform().getOrientation());
}

void PhysicsObject::AddCollider(COLLIDER_TYPE type, glm::vec3 colliderAppearace, glm::vec3 position_vec3, glm::vec3 eulerRotation) {

	switch (type) {
	case BOX: {
		Transform transform = Vec3ToRp3dTransform(position_vec3, eulerRotation);
		BoxShape* boxShape = PhysicsManager::GetInstance().GetPhysicsCommon().createBoxShape(Vec3Convert(colliderAppearace));
		colliders.push_back(body->addCollider(boxShape, transform));
		break;
	}
	case SPHERE: {
		Transform transform = Vec3ToRp3dTransform(position_vec3, eulerRotation);
		SphereShape* sphereShape = PhysicsManager::GetInstance().GetPhysicsCommon().createSphereShape(colliderAppearace.x);
		colliders.push_back(body->addCollider(sphereShape, transform));
		break;
	}
	case CAPSULE: {
		Transform transform = Vec3ToRp3dTransform(position_vec3, eulerRotation);
		CapsuleShape* capsuleShape = PhysicsManager::GetInstance().GetPhysicsCommon().createCapsuleShape(colliderAppearace.x, colliderAppearace.y);
		colliders.push_back(body->addCollider(capsuleShape, transform));
		break;
	}
	default: return;
	}
}

void PhysicsObject::SetCollisionActive(bool isEnabled) {
	for (auto& collider : colliders) {
		collider->setIsSimulationCollider(isEnabled);
	}
}

bool PhysicsObject::GetCollisionActive() {
	if (!colliders.empty())
		return colliders[0]->getIsSimulationCollider();
}

void PhysicsObject::SetTrigger(bool isEnabled) {
	for (auto& collider : colliders) {
		collider->setIsTrigger(isEnabled);
	}
}

bool PhysicsObject::GetIsTrigger() {
	if (!colliders.empty())
		return colliders[0]->getIsTrigger();
}

void PhysicsObject::SetMaterial(float bounciness, float massDensity, float frictionCoefficient, int colliderIndex) {
	bounciness = Clamp(bounciness, 0, 1);
	if (massDensity <= 0)
		massDensity = 0.000001f;
	frictionCoefficient = Clamp(frictionCoefficient, 0, 1);

	if (colliderIndex == -1)
		for (auto& collider : colliders) {
			Material& material = collider->getMaterial();
			material.setBounciness(bounciness);
			material.setMassDensity(massDensity);
			material.setFrictionCoefficient(frictionCoefficient);
		}
	else if (colliderIndex < colliders.size()) {
		Material& material = colliders[colliderIndex]->getMaterial();
		material.setBounciness(bounciness);
		material.setMassDensity(massDensity);
		material.setFrictionCoefficient(frictionCoefficient);
	}
	else
		Error("PhysicsObject::SetMaterial(): invalide colliderIndex");
}

void PhysicsObject::SetBounciness(float bounciness, int colliderIndex) {
	bounciness = Clamp(bounciness, 0, 1);

	if (colliderIndex == -1)
		for (auto& collider : colliders) {
			Material& material = collider->getMaterial();
			material.setBounciness(bounciness);
		}
	else if (colliderIndex < colliders.size()) {
		Material& material = colliders[colliderIndex]->getMaterial();
		material.setBounciness(bounciness);
	}
	else
		Error("PhysicsObject::SetBounciness(): invalide colliderIndex");
}

void PhysicsObject::SetMassDensity(float massDensity, int colliderIndex) {
	if (massDensity <= 0)
		massDensity = 0.000001f;

	if (colliderIndex == -1)
		for (auto& collider : colliders) {
			Material& material = collider->getMaterial();
			material.setMassDensity(massDensity);
		}
	else if (colliderIndex < colliders.size()) {
		Material& material = colliders[colliderIndex]->getMaterial();
		material.setMassDensity(massDensity);
	}
	else
		Error("PhysicsObject::SetMassDensity(): invalide colliderIndex");
}

void PhysicsObject::SetFrictionCoefficient(float frictionCoefficient, int colliderIndex) {
	frictionCoefficient = Clamp(frictionCoefficient, 0, 1);

	if (colliderIndex == -1)
		for (auto& collider : colliders) {
			Material& material = collider->getMaterial();
			material.setFrictionCoefficient(frictionCoefficient);
		}
	else if (colliderIndex < colliders.size()) {
		Material& material = colliders[colliderIndex]->getMaterial();
		material.setFrictionCoefficient(frictionCoefficient);
	}
	else
		Error("PhysicsObject::SetFrictionCoefficient(): invalide colliderIndex");
}

PhysicsObject::PhysicsObject(BODY_TYPE type, glm::vec3 position_vec3, glm::vec3 eulerRotation) {
	Transform transform = Vec3ToRp3dTransform(position_vec3, eulerRotation);
	currentTransform = transform;
	body = PhysicsManager::GetInstance().GetWorld()->createRigidBody(transform);
	this->type = type;
	body->setType(static_cast<rp3d::BodyType>(type));
	body->setLinearDamping(0.3f);
	body->setAngularDamping(0.01f);
	body->setIsDebugEnabled(true);
}

PhysicsObject::~PhysicsObject() {
	PhysicsManager::GetInstance().GetWorld()->destroyRigidBody(body);
}


/********************************* PhysicsEventListener *********************************/

PhysicsEventListener::~PhysicsEventListener() {}

void PhysicsEventListener::onContact(const CollisionCallback::CallbackData& callbackData) {

	for (int i = 0; i < callbackData.getNbContactPairs(); i++) {
		const auto& contactPair = callbackData.getContactPair(i);

		for (auto& event : contactEvents) {
			if (event.physics->Getbody() == contactPair.getBody1() && event.contactType == contactPair.getEventType()) {
				event.event.Invoke(contactPair.getBody1());
			}
			else if (event.physics->Getbody() == contactPair.getBody2() && event.contactType == contactPair.getEventType()) {
				event.event.Invoke(contactPair.getBody2());
			}
		}
	}

}

void PhysicsEventListener::onTrigger(const OverlapCallback::CallbackData& callbackData) {

	for (int i = 0; i < callbackData.getNbOverlappingPairs(); i++) {
		const auto& overlappingPair = callbackData.getOverlappingPair(i);

		for (auto& event : triggerEvents) {
			if (event.physics->Getbody() == overlappingPair.getBody1() && event.overlapType == overlappingPair.getEventType()) {
				event.event.Invoke(overlappingPair.getBody1());
			}
			else if (event.physics->Getbody() == overlappingPair.getBody2() && event.overlapType == overlappingPair.getEventType()) {
				event.event.Invoke(overlappingPair.getBody2());
			}
		}
	}

}

void PhysicsEventListener::AddToContactEvents(PhysicsEvent physicsEvent) {
	if (physicsEvent.event.lock)
		return;

	auto it = std::find_if(contactEvents.begin(), contactEvents.end(), [&](const PhysicsEvent& event) {
		return physicsEvent.physics == event.physics;
		});
	if (it != contactEvents.end()) {
		contactEvents.erase(it);
	}

	contactEvents.push_back(physicsEvent);
}

void PhysicsEventListener::AddToTriggerEvents(PhysicsEvent physicsEvent) {
	if (physicsEvent.event.lock)
		return;

	auto it = std::find_if(triggerEvents.begin(), triggerEvents.end(), [&](const PhysicsEvent& event) {
		return physicsEvent.physics == event.physics;
		});
	if (it != triggerEvents.end()) {
		triggerEvents.erase(it);
	}

	triggerEvents.push_back(physicsEvent);
}

void PhysicsEventListener::UpdateEventValidity(const rp3d::PhysicsWorld* world) {
	for (unsigned i = 0; i < contactEvents.size(); ) {
		if (world->getRigidBody(contactEvents[i].physics->Getbody()->getEntity().id) == nullptr) {
			contactEvents.erase(contactEvents.begin() + i);
			continue;
		}
		i++;
	}
	for (unsigned i = 0; i < triggerEvents.size(); ) {
		if (world->getRigidBody(triggerEvents[i].physics->Getbody()->getEntity().id) == nullptr) {
			triggerEvents.erase(triggerEvents.begin() + i);
			continue;
		}
		i++;
	}
}


/********************************* PhysicsManager *********************************/

void PhysicsManager::InitWorld() {
	world = physicsCommon.createPhysicsWorld(worldSettings);
	world->setEventListener(&eventListener);
	world->setSleepLinearVelocity(0.01f);
	world->setSleepAngularVelocity(0.01f);
	world->setTimeBeforeSleep(1);
}

void PhysicsManager::CleanUp() {
	physicsCommon.destroyPhysicsWorld(world);
}

void PhysicsManager::SetUpLogger(std::string name) {
	// Create the default logger
	logger = physicsCommon.createDefaultLogger();

	// Log level (warnings and errors)
	uint logLevel = static_cast<uint>(static_cast<uint>(Logger::Level::Warning) | static_cast<uint>(Logger::Level::Error));

	// Output the logs into an HTML file
	logger->addFileDestination(directoryLogger + "rp3d_log_" + name + ".html", logLevel, DefaultLogger::Format::HTML);

	// Output the logs into the standard output
	logger->addStreamDestination(std::cout, logLevel, DefaultLogger::Format::Text);

	// Set the logger
	physicsCommon.setLogger(logger);
}

void PhysicsManager::SeteDebugRendering(bool isEnabled) {
	world->setIsDebugRenderingEnabled(isEnabled);
	debugRendering = isEnabled;

	if (isEnabled) {
		DebugRenderer& debugRenderer_reference = world->getDebugRenderer();
		debugRenderer = &debugRenderer_reference;
	}
	else {
		debugRenderer = nullptr;
	}
}

void PhysicsManager::SetDebugRenderItems(bool collisionShape, bool colliderBroadphaseAABB, bool colliderAABB, bool contactPoint, bool contactNormal) {
	if (debugRenderer) {
		debugRenderer->setIsDebugItemDisplayed(DebugRenderer::DebugItem::COLLISION_SHAPE, collisionShape);
		debugRenderer->setIsDebugItemDisplayed(DebugRenderer::DebugItem::COLLIDER_BROADPHASE_AABB, colliderBroadphaseAABB);
		debugRenderer->setIsDebugItemDisplayed(DebugRenderer::DebugItem::COLLIDER_AABB, colliderAABB);
		debugRenderer->setIsDebugItemDisplayed(DebugRenderer::DebugItem::CONTACT_POINT, contactPoint);
		debugRenderer->setIsDebugItemDisplayed(DebugRenderer::DebugItem::CONTACT_NORMAL, contactNormal);
	}
}

void PhysicsManager::UpdatePhysics(double dt) {
	timeAccumulator += dt;

	// use this so that it always runs at constant step while keeping ralatively true to framerate
	while (timeAccumulator >= TIME_STEP) {
		world->update(TIME_STEP);
		timeAccumulator -= TIME_STEP;
	}
}

