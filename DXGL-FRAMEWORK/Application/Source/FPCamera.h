#ifndef FP_CAMERA_H
#define FP_CAMERA_H

// GLM Headers
#include <glm\glm.hpp>

/* required functions for minimum functionality:
* Init
* VariableRefresh (put at the start of scene update)
* Update
*/

class FPCamera
{
public:

    inline const glm::vec3& GetFinalPosition() { return finalPosition; }
    inline const glm::vec3& GetFinalTarget() { return finalTarget; }
    inline const glm::vec3& GetFinalUp() { return finalUp; }
    inline const glm::vec3& GetFinalDirection() { return finalDirection; }

    // return the camera position without bobbing and shaking
    inline const  glm::vec3& GetPlainPosition() { return smoothPosition; }
    // return the camera target without bobbing and shaking
    inline const  glm::vec3& GetPlainTarget() { return smoothTarget; }
    // return the cameraup without bobbing and shaking
    inline const  glm::vec3& GetPlainUp() { return up; }
    // return the camera direction without bobbing and shaking
    inline const glm::vec3& GetPlainDirection() { return direction; }

    // transport the whole of camera to this position with basePosition as the reference point
    // also can transport smoothPosition
    inline void TransportTo(glm::vec3 newPosition) {
        smoothPosition += newPosition - basePosition;
        basePosition = newPosition;
    }

    glm::vec3 basePosition;
    // psi gets reset at the start of update
    float psi;

    glm::vec3 ViewIntersectXZPlane;

    float movePositionOffset = 0;

    float bobbingSpeed = 1;
    float bobbingSmoothSpeed = bobbingSpeed;
    float bobbingMaxX = 0.12f;
    float bobbingMaxY = 0.085f;
    float bobbingMaxPsi = 1.25f;

    // toggle if any movements of camera can be done through camera's update
    bool bobbingActive = false;

    FPCamera();
    ~FPCamera();

    void Init(glm::vec3 position, glm::vec3 direction);
    void Reset();
    void Update(double dt);

    // force recalcuation of camera stats in the next Update
    void ForceNextRefresh();

    // recalculate camera stats right now
    void ForceRefresh();

    void SetDirection(glm::vec3 direction, float rollAngleDeg = 0);

    // refresh variables with initial default values per frame
    void VariableRefresh();

    enum class MODE {
        FREE,
        FIRST_PERSON,
        PAUSE,
        LOCK_ON,

        TOTAL_CAMERA_MODE,
    };

    MODE currentMode = MODE::FREE;

    inline const MODE& GetCurrentMode() { return currentMode; }
    void Set(MODE mode);

private:

    bool allowSelfMovement = true;
    bool allowPositionMovement = false;
    bool allowViewRotation = true;

    glm::vec3 finalPosition;
    glm::vec3 finalTarget;
    glm::vec3 finalUp;
    glm::vec3 finalDirection;

    glm::vec3 smoothPosition;
    glm::vec3 smoothTarget;

    glm::vec3 direction;
    glm::vec3 up;

    float phi;
    float theta;

    float rotSmoothing = 10;
    float moveSmoothing = 20;
    float smoothPhi;
    float smoothTheta;
    float smoothPsi;

    float bobbingX = 0;
    float bobbingY = 0;
    float bobbingPsi = 0;
    float bobbingElapsed = 0;

    bool isDirty;   // indicate if there is a need to recalculate the camera attributes

    void Refresh();
};

#endif