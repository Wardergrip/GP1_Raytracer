#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix.h"
#include "ColorRGB.h"
#include "MathHelpers.h"

namespace helperFuncts
{
	inline dae::Vector3 HalfVector(const dae::Vector3& light, const dae::Vector3& view)
	{
		return ((view + light) / (view + light).Magnitude());
	}
}