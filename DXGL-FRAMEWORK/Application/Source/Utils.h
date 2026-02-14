//chinykian
//last updated: 20 Oct 2024
#pragma once

#include "Vertex.h"
#include "Vector3.h"

Color HexToColor(unsigned hexColor);

// JH
float clamp(float value, float min, float max);
float vecToThetaDeg(Vector3 vec);
Vector3 thetaDegToVec(float theta);
unsigned mod(int divident, int divisor);
Vector3 rotateOffset(const Vector3& offset, float angleRad);