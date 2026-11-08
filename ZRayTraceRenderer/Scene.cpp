#include "Scene.h"

Scene::~Scene()
{
    for (auto& obj : mSceneObjects)
    {
        delete obj;
    }
    for (auto& light : mLights)
    {
        delete light;
    }
    for (auto& pair : mMatrials)
    {
        delete pair.second;
    }
	for (auto& pair : mTextures)
	{
		delete pair.second;
	}
}

SceneObject* Scene::CreateSceneObject(const Vector3f& position, const Vector3f& euler, float scale)
{
    SceneObject* pSceneObject = new SceneObject(position, euler, scale);
    mSceneObjects.push_back(pSceneObject);
    return pSceneObject;
}

Texture* Scene::GetOrCreateTexture(const std::string& filePath)
{
	if (mTextures.find(filePath) != mTextures.end())
	{
		return mTextures[filePath];
	}

	Texture* pTexture = Texture::CreateFromFile(filePath);
	if (pTexture == nullptr)
		return nullptr;

	mTextures[filePath] = pTexture;
	return pTexture;
}

SceneObject* Scene::Intersect(Ray& ray, Intersection& isect)
{
    SceneObject* pHitObject = nullptr;
    for (const auto& pSceneObject : mSceneObjects)
    {
        if (pSceneObject->Intersect(ray, isect))
        {
            ray.maxt = isect.t;
            pHitObject = pSceneObject;
        }
    }
    return pHitObject;
}

bool Scene::LoadFromXML(const char* filename, int W, int H)
{
    SceneLoader loader;

    if (!loader.LoadFile(filename))
        return false;

    if (!loader.LoadCamera(mCamera, W, H))
        return false;

    if (!loader.LoadMaterials(*this))
        return false;

    if (!loader.LoadSceneObjects(*this))
        return false;

    if (!loader.LoadLights(*this))
        return false;



    return true;
}
