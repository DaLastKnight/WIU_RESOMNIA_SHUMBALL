// JH
#ifndef UTILS_H
#define UTILS_H

#include <reactphysics3d/reactphysics3d.h>
#include <glm\glm.hpp>
#include <glm\gtc\quaternion.hpp>
#include <string>


inline float Vec3LengthSqred(glm::vec3 vec) {
	return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

inline float Clamp(float value, float min, float max) {
	if (value < min)
		return min;
	else if (value > max)
		return max;
	return value;
}

inline float Wrap(float value, float min, float max) {
	if (value < min)
		return value + (max - min);
	else if (value > max)
		return value - (max - min);
	return  value;
}

// can mod nagative divident
inline unsigned NegativeMod(int divident, int divisor) {
	while (divident < 0)
		divident += divisor;
	return divident % divisor;
}

inline std::string VecToString(glm::vec2 vec) {
	return "[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + "] ";
}
inline std::string VecToString(glm::vec3 vec) {
	return "[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + "] ";
}
inline std::string VecToString(glm::vec4 vec) {
	return "[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ", " + std::to_string(vec.w) + "] ";
}

// Smoothing >= 1, 1 means no smoothing, largers the smoothing, the longer current takes to reach target
// type must support / and * with float, and + and - with its own type
template<typename T>
inline T Smooth(const T& currentValue, const T& targetValue, float Smoothing, float deltaTime) {
	return currentValue + (targetValue - currentValue) / Smoothing * deltaTime * 100.f;
}

inline rp3d::Vector3 Vec3Convert(glm::vec3 vec3) {
	return rp3d::Vector3(vec3.x, vec3.y, vec3.z);
}
inline glm::vec3 Vec3Convert(rp3d::Vector3 vector3) {
	return glm::vec3(vector3.x, vector3.y, vector3.z);
}

inline rp3d::Quaternion EulerToQuaternion(glm::vec3 eulerVec3) {
	rp3d::Quaternion qx(rp3d::Vector3(1, 0, 0), glm::radians(eulerVec3.x));
	rp3d::Quaternion qy(rp3d::Vector3(0, 1, 0), glm::radians(eulerVec3.y));
	rp3d::Quaternion qz(rp3d::Vector3(0, 0, 1), glm::radians(eulerVec3.z));
	return qz * qy * qx;
}

inline rp3d::Transform Vec3ToRp3dTransform(glm::vec3 position_vec3, glm::vec3 eulerRotation) {
	rp3d::Vector3 position = Vec3Convert(position_vec3);
	rp3d::Quaternion orientation = EulerToQuaternion(eulerRotation);
	return rp3d::Transform(position, orientation);
}

inline glm::vec3 HexToVec3(uint32_t color) {
	float r = ((color >> 24) & 0xFF) / 255.0f;
	float g = ((color >> 16) & 0xFF) / 255.0f;
	float b = ((color >> 8) & 0xFF) / 255.0f;
	return { r, g, b };
}

inline glm::vec3 QuaternionToEuler(rp3d::Quaternion orientation) {
	return glm::degrees(glm::eulerAngles(glm::quat(orientation.w, orientation.x, orientation.y, orientation.z)));
}
inline glm::vec3 QuatToEuler(glm::quat orientation) {
	return glm::degrees(glm::eulerAngles(orientation));
}

inline glm::quat QuaternionToQuat(rp3d::Quaternion orientation) {
	return glm::quat(orientation.w, orientation.x, orientation.y, orientation.z);
}

#endif
