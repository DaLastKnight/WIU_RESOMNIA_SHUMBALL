// JH
#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include <reactphysics3d/reactphysics3d.h>
#include <glm/glm.hpp>

#include <string>

#include "Utils.h"

/* notes:
* all pointers returned by reactphysics3d shall not be manually freed through delete, as the lib is responsible for all memory allocation from it, the PhysicsCommon class will manage the memory on its own
*/


class PhysicsEventListener : public rp3d::EventListener {
public:

    ~PhysicsEventListener();
    void onContact(const rp3d::CollisionCallback::CallbackData& callbackData) override;
    void onTrigger(const rp3d::OverlapCallback::CallbackData& callbackData) override;

    inline const rp3d::CollisionCallback::CallbackData* GetContectData();
    inline const rp3d::OverlapCallback::CallbackData* GetTriggerData();

private:
    const rp3d::CollisionCallback::CallbackData* ContactData;
    const rp3d::OverlapCallback::CallbackData* TriggerData;
};


class PhysicsManager {
public:

    
	
    static PhysicsManager& GetInstance() {
        static PhysicsManager physicsManager;
        return physicsManager;
    }

    void InitWorld();
    void CleanUp();

    rp3d::PhysicsCommon& GetPhysicsCommon() {
        return physicsCommon;
    }
    rp3d::PhysicsWorld* GetWorld() {
        return world;
    }
    const double& GetTimeAccumulator() {
        return timeAccumulator;
    }
    const double& Get_TIME_STEP() {
        return TIME_STEP;
    }
    const PhysicsEventListener& GetEventListener() {
        return eventListener;
    }
    rp3d::DebugRenderer* GetDebugRenderer() {
        return debugRenderer;
    }

    void SetUpLogger(std::string name);
    void SeteDebugRendering(bool isEnabled);
    void SetDebugRenderItems(bool collisionShape, bool colliderBroadphaseAABB, bool colliderAABB, bool contactPoint, bool contactNormal);

    void UpdatePhysics(double dt);


private:

    rp3d::PhysicsCommon physicsCommon;

    rp3d::PhysicsWorld* world;

    static constexpr double TIME_STEP = 1 / 60.0;
    double timeAccumulator = 0;

    rp3d::DefaultLogger* logger;
    std::string directoryLogger = "Log/ReactPhysics3D/";
    rp3d::DebugRenderer* debugRenderer;

    bool debugRendering = false;

    PhysicsEventListener eventListener;

    PhysicsManager() = default;
    ~PhysicsManager() = default;
    PhysicsManager(const PhysicsManager&) = delete;
    PhysicsManager& operator=(const PhysicsManager&) = delete;
};


class PhysicsObject {
public:

    // static: infinite mass with no movement
    // kinematic: infinite mass with movement
    // dynamic: finite mass with movement
    enum BODY_TYPE {
        STATIC = static_cast<BODY_TYPE>(rp3d::BodyType::STATIC),
        KINEMATIC = static_cast<BODY_TYPE>(rp3d::BodyType::KINEMATIC),
        DYNAMIC = static_cast<BODY_TYPE>(rp3d::BodyType::DYNAMIC),
    };

    enum COLLIDER_TYPE {
        BOX,
        SPHERE,
        CAPSULE
    };

    void AddForce(glm::vec3 force) {
        body->applyLocalForceAtCenterOfMass(Vec3Convert(force));
    }
    void AddImpulse(glm::vec3 force) {
        body->setLinearVelocity(rp3d::Vector3(0, 0, 0));
        body->applyLocalForceAtCenterOfMass(Vec3Convert(force) * PhysicsManager::GetInstance().Get_TIME_STEP());
    }
    void AddSoftImpulse(glm::vec3 impulse) {
        body->applyLocalForceAtCenterOfMass(Vec3Convert(impulse) * PhysicsManager::GetInstance().Get_TIME_STEP());
    }

    void SetVelocity(glm::vec3 velocity) {
        body->setLinearVelocity(Vec3Convert(velocity));
    }
    void SetAngularVelocity(glm::vec3 velocity) {
        body->setAngularVelocity(Vec3Convert(velocity));
    }

    void SetAllowedMovementAxes(glm::vec3 axes) {
        body->setLinearLockAxisFactor(Vec3Convert(axes));
    }
    void SetAllowedRotationAxes(glm::vec3 axes) {
        body->setAngularLockAxisFactor(Vec3Convert(axes));
    }

    void SetTransform(glm::vec3 position_vec3 = glm::vec3(0), glm::vec3 eulerRotation = glm::vec3(0));
    void SetPosition(glm::vec3 position_vec3);
    void SetOrientation(glm::vec3 eulerRotation);
    void InterpolateTransform();
    glm::mat4 GetModel();
    glm::vec3 GetPostion();
    glm::quat GetOrientation();

    // colliderAppearace |
    // if type is BOX: colliderAppearace is the half dimension size
    // if type is SPHERE: x is the radius
    // if type is CAPSULE: x is radius, y is total height - radius * 2
    void AddCollider(COLLIDER_TYPE type, glm::vec3 colliderAppearace, glm::vec3 position_vec3 = glm::vec3(0), glm::vec3 eulerRotation = glm::vec3(0));
    std::vector<rp3d::Collider*> GetColliders() {
        return colliders;
    }

    void SetCollisionActive(bool isEnabled); // collission for physics
    bool GetCollisionActive(); // collission for physics

    void SetTrigger(bool isEnabled); // set if only detect collision, also set CollisionActive as opposite
    bool GetIsTrigger(); // detect collision

    void SetMaterial(float bounciness, float massDensity, float frictionCoefficient, int colliderIndex = -1);
    void SetBounciness(float bounciness, int colliderIndex = -1);
    void SetMassDensity(float massDensity, int colliderIndex = -1);
    void SetFrictionCoefficient(float frictionCoefficient, int colliderIndex = -1);
    void UpdateMassProperties() {
        body->updateMassPropertiesFromColliders();
    }

    inline void SetAllowSleep(bool allow) {
        body->setIsAllowedToSleep(allow);
    }
    inline void SetIsSleeping(bool isSleeping) {
        body->setIsSleeping(isSleeping);
    }

    PhysicsObject(BODY_TYPE type, glm::vec3 position_vec3 = glm::vec3(0), glm::vec3 eulerRotation = glm::vec3(0));
    ~PhysicsObject();

private:

    BODY_TYPE type;
    rp3d::RigidBody* body;
    std::vector<rp3d::Collider*> colliders;
    rp3d::Transform currentTransform;

};

#endif
