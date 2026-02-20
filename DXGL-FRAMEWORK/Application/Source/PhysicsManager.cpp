#include "PhysicsManager.h"

#include <glm/gtc/type_ptr.hpp>
#include "Console.h"

using namespace reactphysics3d;


/********************************* PhysicsEventListener *********************************/

PhysicsEventListener::~PhysicsEventListener() {}

void PhysicsEventListener::onContact(const CollisionCallback::CallbackData& callbackData) {

	ContactData = &callbackData;

	//// For each contact pair
	//for (uint p = 0; p < callbackData.getNbContactPairs(); p++) {
	//	// Get the contact pair
	//	CollisionCallback::ContactPair contactPair = callbackData.getContactPair(p);
	//	// For each contact point of the contact pair
	//	for (uint c = 0; c < contactPair.getNbContactPoints(); c++) {
	//		// Get the contact point
	//		CollisionCallback::ContactPoint contactPoint = contactPair.getContactPoint(c);
	//		// Get the contact point on the first collider and convert it in world-space
	//		Vector3 worldPoint = contactPair.getCollider1()->getLocalToWorldTransform() * contactPoint.getLocalPointOnCollider1();
	//	}
	//}
}

void PhysicsEventListener::onTrigger(const OverlapCallback::CallbackData& callbackData) {
	
	TriggerData = &callbackData;
	
	//// For each contact pair
	//for (uint p = 0; p < callbackData.getNbOverlappingPairs(); p++) {
	//	// Get the contact pair
	//	OverlapCallback::OverlapPair overlapPair = callbackData.getOverlappingPair(p);
	//}
}

inline const rp3d::CollisionCallback::CallbackData* PhysicsEventListener::GetContectData() {
	return ContactData;
}

inline const rp3d::OverlapCallback::CallbackData* PhysicsEventListener::GetTriggerData() {
	return TriggerData;
}


/********************************* PhysicsManager *********************************/

void PhysicsManager::InitWorld() {
	PhysicsWorld::WorldSettings settings; // change settings with this if needed
	world = physicsCommon.createPhysicsWorld();
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


/********************************* PhysicsObject *********************************/

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

glm::vec3 PhysicsObject::GetPostion() {
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
		body->addCollider(boxShape, transform);
		break;
	}
	case SPHERE: {
		Transform transform = Vec3ToRp3dTransform(position_vec3, eulerRotation);
		SphereShape* sphereShape = PhysicsManager::GetInstance().GetPhysicsCommon().createSphereShape(colliderAppearace.x);
		body->addCollider(sphereShape, transform);
		break;
	}
	case CAPSULE: {
		Transform transform = Vec3ToRp3dTransform(position_vec3, eulerRotation);
		CapsuleShape* capsuleShape = PhysicsManager::GetInstance().GetPhysicsCommon().createCapsuleShape(colliderAppearace.x, colliderAppearace.y);
		body->addCollider(capsuleShape, transform);
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
		collider->setIsSimulationCollider(!isEnabled);
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
	body->setLinearDamping(0.5f);
	body->setAngularDamping(0.01f);
	body->setIsDebugEnabled(true);
}

PhysicsObject::~PhysicsObject() {
	PhysicsManager::GetInstance().GetWorld()->destroyRigidBody(body);
}
