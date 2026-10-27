#pragma once
#include "Ray.h"
class Triangle
{
public:
	// 传入 模型空间的点  世界变换的矩阵 
	Triangle(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Matrix4x4 worldMatrix);
	bool Intersect(Ray& ray, Intersection& isect) const;

private:
	Vector3f mVertices[3];
	Vector3f mNormal;
};

