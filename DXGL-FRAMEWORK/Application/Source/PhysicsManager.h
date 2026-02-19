// JH
#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include <reactphysics3d/reactphysics3d.h>

/* notes:
* all pointers returned by reactphysics3d shall not be manually freed through delete, as the lib is responsible for all memory allocation from it, the PhysicsCommon class will manage the memory on its own
*/

class PhysicsManager {
public:
	
    static PhysicsManager& GetInstance() {
        static PhysicsManager physicsManager;
        return physicsManager;
    }

    inline void InitWorld();

    void UpdatePhysics(double dt);


private:

    rp3d::PhysicsCommon physicsCommon;

    rp3d::PhysicsWorld* world;

    static constexpr double TIME_STEP = 1 / 60.0;
    double timeAccumulator = 0;

    PhysicsManager() = default;
    ~PhysicsManager() = default;
    PhysicsManager(const PhysicsManager&) = delete;
    PhysicsManager& operator=(const PhysicsManager&) = delete;
};

#endif
