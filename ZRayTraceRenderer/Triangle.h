#pragma once
#include "Primitive.h"

class Triangle : public Primitive
{
public:
	// 传入 模型空间的点  世界变换的矩阵 
	Triangle(SceneObject* pSceneObject, const Vector3f positions[], const Vector2f uvs[]);
	virtual bool Intersect(Ray& ray, Intersection& isect) const override;
	virtual void Sample(Vector3f& p, Vector3f& normal, float& pdf) const override;
private:
	Vector3f mPositions[3];
	Vector2f mTexCoords[3];
	Vector3f mNormal;
	float mArea;
};

