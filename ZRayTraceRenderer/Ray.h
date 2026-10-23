#pragma once
#include "Common.h"

// 定义射线
struct Ray
{
	Vector3f o;
	Vector3f d;

	float mint = 0.0f;
	float maxt = FLT_MAX;
};