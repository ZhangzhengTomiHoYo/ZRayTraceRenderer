#include "SceneObject.h"

SceneObject::~SceneObject()
{
	for (auto& primitive : mPrimitives)
	{
		if (primitive)
		{
			delete primitive;
		}
	}
}

SceneObject::SceneObject(const Vector3f& position, const Vector3f& euler, float scale) : mEmissive(0)
{
	mObjectToWorld = makeWorldTransform(position, euler, scale);
	mWorldToObject = glm::inverse(mObjectToWorld);
}

Matrix4x4 SceneObject::GetObejctToWorld() const
{
	return mObjectToWorld;
}

Matrix4x4 SceneObject::GetWorldtoObject() const
{
	return mWorldToObject;
}

bool SceneObject::Intersect(Ray& ray, Intersection& isect)
{
	bool hit = false;
	for (const auto& primitive : mPrimitives)
	{
		if (primitive->Intersect(ray, isect))
		{
			ray.maxt = isect.t;
			hit = true;
		}
	}
	return hit;
}

void SceneObject::Sample(Vector3f& p, Vector3f& normal, float& pdf)
{
	int index = RandomInt(0, mPrimitives.size() - 1);
	mPrimitives[index]->Sample(p, normal, pdf);
	pdf /= mPrimitives.size();
}

//void SceneObject::AddPrimitives(Primitive* primitive)
//{
//	mPrimitives.push_back(primitive);
//}
