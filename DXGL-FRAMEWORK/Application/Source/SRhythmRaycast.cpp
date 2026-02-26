
#include "SRhythmRaycast.h"


rp3d::decimal PhysicsRaycast::notifyRaycastHit(const rp3d::RaycastInfo& raycastInfo) {
    raycastInfos.push_back(raycastInfo);

    type = defaultRaycastEvent.Invoke(raycastInfo);

    if (type == PENETRATION)
        return 1.0;
    else
        return raycastInfo.hitFraction;
}

int PhysicsRaycast::FindHit(rp3d::RigidBody* body) {
    for (int i = 0; i < raycastInfos.size(); i++) {
        auto& info = raycastInfos[i];
        if (info.body == body) {
            return i;
        }
    }
    return -1;
}
