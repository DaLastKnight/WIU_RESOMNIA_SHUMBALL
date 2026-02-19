// JH
#ifndef UTILS_H
#define UTILS_H

#include <glm\glm.hpp>
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

#endif
