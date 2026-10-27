#pragma once
#include "Common.h"
#include "Ray.h"

class Camera
{
public:
	Vector3f mPosition;

	//Matrix4x4 viewMatrix;
	//Matrix4x4 projMatrix;

	Matrix4x4 mCombinedMatrix;            // viewport * proj * view
	Matrix4x4 mInverseCombinedMatrix;     // 

	void Init(const Vector3f& p, const Vector3f& target, const Vector3f& up, float fov, float near, float far, int W, int H);
	Ray GetRay(float x, float y) const;
};

