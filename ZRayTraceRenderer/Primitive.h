#pragma once
#include "Ray.h"
#include "SceneObject.h"

class SceneObject;

class Primitive
{
public:
	Primitive(SceneObject* pSceneObject) : m_pSceneObejct(pSceneObject) {};
	virtual ~Primitive() {};
	virtual bool Intersect(Ray& ray, Intersection& isect) const = 0;
	virtual void Sample(Vector3f& p, Vector3f& normal, float& pdf) const = 0;
protected:
	SceneObject* m_pSceneObejct = nullptr;
};

