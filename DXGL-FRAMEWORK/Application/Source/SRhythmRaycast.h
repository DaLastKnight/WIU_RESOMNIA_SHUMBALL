
#ifndef RHYTHM_RAYCAST_H
#define RHYTHM_RAYCAST_H

#include "PhysicsManager.h"


class PhysicsRaycast : public rp3d::RaycastCallback {
public:

    enum RAYCAST_TYPE {
        PENETRATION,
        STOP_ON_CONTACT
    };

    struct RaycastEvent {

        Event<RAYCAST_TYPE, const rp3d::RaycastInfo&> event;
        PhysicsObject* physics;

        RaycastEvent(PhysicsObject* physics, Event<RAYCAST_TYPE, const rp3d::RaycastInfo&> event)
            : physics(physics), event(event) {}
    };

    struct PhysicsRaycastInfo {
        PhysicsRaycastInfo(const rp3d::RaycastInfo& rcInfo) {
            worldPoint = rcInfo.worldPoint;
            worldNormal = rcInfo.worldNormal;
            hitFraction = rcInfo.hitFraction;
            triangleIndex = rcInfo.triangleIndex;
            body = rcInfo.body;
            collider = rcInfo.collider;
        }

        rp3d::Vector3 worldPoint;
        rp3d::Vector3 worldNormal;
        rp3d::decimal hitFraction;
        int triangleIndex;
        rp3d::Body* body;
        rp3d::Collider* collider;
    };

    Event<RAYCAST_TYPE, const rp3d::RaycastInfo&> defaultRaycastEvent;

    rp3d::decimal notifyRaycastHit(const rp3d::RaycastInfo& raycastInfo) override;

    const std::vector<PhysicsRaycastInfo>& GetRaycastInfos() {
        return raycastInfos;
    }
    void ClearInfo() {
        raycastInfos.clear();
    }
    const RAYCAST_TYPE& GetCurrentRaycastType() {
        return type;
    }

    int FindHit(rp3d::RigidBody* body);
   
private:

    std::vector<PhysicsRaycastInfo> raycastInfos;
    RAYCAST_TYPE type;
};

inline rp3d::Ray MakeRay(glm::vec3 p1 = glm::vec3(0), glm::vec3 p2 = glm::vec3(0), float maxTravelFraction = 1.f) {
    return rp3d::Ray(Vec3Convert(p1), Vec3Convert(p2), maxTravelFraction);
}

inline bool IsSameRay(rp3d::Ray r1, rp3d::Ray r2) {
    return (
        r1.point1 == r2.point1 &&
        r1.point2 == r2.point2 &&
        r1.maxFraction == r2.maxFraction
        );
}

#endif
