#pragma once
#include <vector>
#include <utility>

#include "Ray.h"
#include "Primitive.h"
#include "Material.h"

class Primitive;

// 场景对象：一个场景对象包含多个图元
class SceneObject
{
public:
	~SceneObject();
	SceneObject(const Vector3f& position, const Vector3f& euler, float scale);

	Matrix4x4 GetObejctToWorld() const;
	Matrix4x4 GetWorldtoObject() const;

	bool Intersect(Ray& ray, Intersection& isect);
	//void AddPrimitives(Primitive* primitive);

	template<typename T, typename ...Args>
	T* CreatePrimitive(Args&&... args)
	{
		T* primitive = new T(this, std::forward<Args>(args)...);
		mPrimitives.push_back(primitive);
		return primitive;
	}

	void SetMaterial(Material* pMaterial) { m_pMaterial = pMaterial; }
	Material* GetMaterial() const { return m_pMaterial; }

private:
	Matrix4x4 mObjectToWorld;
	Matrix4x4 mWorldToObject;

	Material* m_pMaterial = nullptr;

	std::vector<Primitive*> mPrimitives;
};

