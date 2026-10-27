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

protected:
	SceneObject* m_pSceneObejct = nullptr;
};

