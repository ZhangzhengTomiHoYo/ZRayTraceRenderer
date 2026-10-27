#pragma once
#include "SceneObject.h"
#include "Camera.h"

class Scene
{
public:
	~Scene();
	void SetCamera(const Camera& camera) { mCamera = camera; };
	const Camera& GetCamera() const { return mCamera; };

	// 工厂方法 + 谁创建谁销毁 = 需要一个统一创建函数
	SceneObject* CreateSceneObject(const Vector3f& position, const Vector3f& euler, float scale);

	SceneObject* Intersect(Ray& ray, Intersection& isect);
private:
	Camera mCamera;
	std::vector<SceneObject*> mSceneObjects;
};

