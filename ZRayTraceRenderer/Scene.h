#pragma once
#include "SceneObject.h"
#include "Camera.h"
#include "SceneLoader.h"
#include "Light.h"

class Scene
{
public:
	~Scene();
	void SetCamera(const Camera& camera) { mCamera = camera; };
	const Camera& GetCamera() const { return mCamera; };

	bool LoadFromXML(const char* filename, int W, int H);

	// 工厂方法 + 谁创建谁销毁 = 需要一个统一创建函数
	SceneObject* CreateSceneObject(const Vector3f& position, const Vector3f& euler, float scale);
	SceneObject* Intersect(Ray& ray, Intersection& isect);

	template<typename T, typename... Args>
	T* CreateLight(Args&&... args)
	{
		Light* light = new T(std::forward<Args>(args)...);
		mLights.push_back(light);
		return static_cast<T*>(light);
	}

	const std::vector<Light*>& GetLights() const { return mLights; }

private:
	Camera mCamera;
	std::vector<SceneObject*> mSceneObjects;

	std::vector<Light*> mLights;
};

