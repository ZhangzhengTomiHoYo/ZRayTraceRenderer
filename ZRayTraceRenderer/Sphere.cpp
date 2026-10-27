#include "Sphere.h"

Sphere::Sphere(const Vector3f& center, float R) : mRadius(R)
{
	mObjectToWorld = makeWorldTransform(center, Vector3f(0, 0, 0), 1.0f);
	mWorldToObject = glm::inverse(mObjectToWorld);
}

bool Sphere::Intersect(Ray& ray, Intersection& isect) const
{
	Ray localRay = mWorldToObject * ray;

	float A = glm::dot(localRay.d, localRay.d);
	float B = glm::dot(localRay.o, localRay.d) * 2.0f;
	float C = glm::dot(localRay.o, localRay.o) - mRadius * mRadius;

	float delta = B * B - 4 * A * C;

	if (delta < 0) return false;

	float sqrtDelta = sqrtf(delta);
	float t1 = (-B - sqrtDelta) / (2.0f * A);
	float t2 = (-B + sqrtDelta) / (2.0f * A);

	// 落在两边直接排除
	if (localRay.maxt < t1 || localRay.mint > t2) return false;

	// 先令t=t1 然后逐步排查
	float t = t1;
	if (t < localRay.mint)
	{
		t = t2;
		if (t > localRay.maxt)
		{
			return false;
		}
	}

	// 求交点
	Vector3f p = localRay.o + t * localRay.d;
	Vector3f n = glm::normalize(p);
	isect.position = Vector3f(mObjectToWorld * Vector4f(p, 1.0));
	isect.normal = Vector3f(mObjectToWorld * Vector4f(n, 0.0));
	isect.t = t;

	return true;
}
