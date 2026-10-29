#pragma once
#include <map>
#include <string>

#include "SceneObject.h"
#include "Camera.h"
#include "SceneLoader.h"
#include "Light.h"
#include "Material.h"

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
	T* CreateLight( Args&&... args)
	{
		Light* light = new T(std::forward<Args>(args)...);
		mLights.push_back(light);
		return static_cast<T*>(light);
	}

	template<typename T, typename... Args>
	T* CreateMatrial(const std::string& name, Args&&... args)
	{
		T* material = new T(std::forward<Args>(args)...);
		mMatrials.insert({name, material});
		return material;
	}

	const std::vector<Light*>& GetLights() const { return mLights; }
	const std::map<std::string, Material*>& GetMaterials() const { return mMatrials; }

private:
	Camera mCamera;
	std::vector<SceneObject*> mSceneObjects;
	std::vector<Light*> mLights;
	std::map<std::string, Material*> mMatrials;
};

