#include "Disk.h"


Disk::Disk(SceneObject* pSceneObject, float radius) : Primitive(pSceneObject), mRadius(radius)
{
}

bool Disk::Intersect(Ray& ray, Intersection& isect) const
{
	Ray localRay = m_pSceneObejct->GetWorldtoObject() * ray;

	// 数学意义：检查分母是否为0
	// 几何意义：射线 z 轴的分量很小 所以近乎平行 z 轴 没有交点
	if (fabs(localRay.d.z) < 1e-6)
	{
		return false;
	}
	float t = -localRay.o.z / localRay.d.z;
	if (t < localRay.mint || t > localRay.maxt)
	{
		return false;
	}

	Vector3f p = localRay.o + t * localRay.d;

	// 取模大于半径 位于圆盘外 没有交点
	if (glm::length(p) > mRadius)
	{
		return false;
	}

	isect.position = Vector3f(m_pSceneObejct->GetObejctToWorld() * Vector4f(p, 1.0));
	isect.normal = glm::normalize(Vector3f(m_pSceneObejct->GetObejctToWorld() * Vector4f(0,0,1,0)));
	isect.t = t;

	return true;
}

