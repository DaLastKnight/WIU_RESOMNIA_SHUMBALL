//chinykian 
//last updated: 20 Oct 2024
#include "Utils.h"

Color HexToColor(unsigned hexColor)
{
	return Color{ ((hexColor >> 16) & 0xFF) / 255.f,
				  ((hexColor >> 8) & 0xFF) / 255.f,
				  (hexColor & 0xFF) / 255.f };
}

// JH 
float clamp(float value, float min, float max) {
    if (value < min)
        return min;
    else if (value > max)
        return max;
    return value;
}

float vecToThetaDeg(Vector3 vec) {
    return atan2f(vec.y, vec.x) * (180 / Math::PI);
}

Vector3 thetaDegToVec(float theta) {
    return Vector3(cos(theta * (Math::PI / 180)), sin(theta * (Math::PI / 180)));
}

unsigned mod(int divident, int divisor) {
    while (divident < 0)
        divident += divisor;
    return divident % divisor;
}

Vector3 rotateOffset(const Vector3& offset, float thetaRad) {
    float cos = cosf(thetaRad);
    float sin = sinf(thetaRad);
    return Vector3(
        offset.x * cos - offset.y * sin,
        offset.x * sin + offset.y * cos,
        offset.z
    );
}