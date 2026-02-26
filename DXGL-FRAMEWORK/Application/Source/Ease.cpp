
#define _USE_MATH_DEFINES
#include <cmath>

#include "Ease.h"

float Ease(EASE ease, float t) {

	switch (ease) {
	case EASE::LINEAR:
		return t;

	case EASE::IN_SIN:
		return 1 - cosf(t * M_PI_2);
		
	case EASE::OUT_SIN:
		return sinf(t * M_PI_2);
		
	case EASE::IN_OUT_SIN:
		return 0.5f * (1.0f - cosf(t * M_PI));

	case EASE::IN_QUAD:
		return t * t;
		
	case EASE::OUT_QUAD:
		return 1 - (1 - t) * (1 - t);

	case EASE::IN_OUT_QUAD:
		if (t < 0.5f)
			return 2 * t * t;
		else
			return 1 - ((-2 * t + 2) * (-2 * t + 2)) / 2;

	case EASE::IN_ELASTIC: {
		float a = 1.f; // amplitude
		float p = 0.3f; // period
		float s = p / 4.f; // shift
		return -(a * powf(2.f, 10.f * (t - 1))) * sinf((t - 1 - s) * ((2.f * M_PI) / p));
	}
	case EASE::OUT_ELASTIC: {
		float a = 1.0f;
		float p = 0.3f;
		float s = p / 4.0f;
		return (a * powf(2.f, -10.f * t) * sinf((t - s) * (2.f * M_PI) / p)) + 1.f;
	}
	case EASE::IN_OUT_ELASTIC: {
		float a = 1.0f;
		float p = 0.3f;
		float s = p / 4.0f;
		if (t < 0.5f)
			return -0.5f * (powf(2.f, 10.f * (2.f * t - 1.f)) * sinf(((2.f * t - 1.f) - s) * (2.f * M_PI) / p));
		else
			return 0.5f * (powf(2.f, -10.f * (2.f * t - 1.f)) * sinf(((2.f * t - 1.f) - s) * (2.f * M_PI) / p)) + 1.f;
	}
	case EASE::IN_BACK: {
		float s = 1.70158f; // strength
		return t * t * ((s + 1) * t - s);
	}
	case EASE::OUT_BACK: {
		float s = 1.70158f;
		float adjustedT = t - 1.0f;
		return adjustedT * adjustedT * ((s + 1) * adjustedT + s) + 1.0f;
	}
	case EASE::IN_OUT_BACK: {
		float s = 1.70158f * 1.525f; // increase strength so its not too weak visually
		if (t < 0.5f) {
			float adjustedT = 2.0f * t;
			return 0.5f * (adjustedT * adjustedT * ((s + 1) * adjustedT - s));
		}
		else {
			float adjustedT = 2.0f * t - 2.0f;
			return 0.5f * (adjustedT * adjustedT * ((s + 1) * adjustedT + s) + 2.0f);
		}
	}
	case EASE::IN_QUINT:
		return t * t * t * t * t;
		
	case EASE::OUT_QUINT: {
		double adjustedT = (t - 1);
		return adjustedT * adjustedT * adjustedT * adjustedT * adjustedT + 1;
	}
	case EASE::IN_OUT_QUINT:
		if (t < 0.5f)
			return 2 * t * t * t * t * t;
		else {
			double adjustedT = (2 * t) - 2;
			return 0.5 * adjustedT * adjustedT * adjustedT * adjustedT * adjustedT + 1;
		}
	default: return -1;
	}
}
