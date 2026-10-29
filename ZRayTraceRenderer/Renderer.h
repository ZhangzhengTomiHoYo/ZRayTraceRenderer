#pragma once
#include <atomic>
#include <vector>
#include "Common.h"
#include "Camera.h"
#include "Sphere.h"
#include "Disk.h"
#include "Triangle.h"
#include "Scene.h"

class Renderer
{
public:
	Renderer();
	Renderer(const char* filename, int minDepth, int maxDepth, int width, int height);
	~Renderer();
	void Run();

	Color GetIrridance(Ray& ray);
	Color GetRadiance(Ray& ray, int depth);


private:
	Color RenderPixel(int x, int y);
	Color RenderSubPixel(float x, float y);
	void RunRenderThread();
	int mViewportWidth;
	int mViewportHeight;
	uint32_t* mBuffer;
	std::atomic<int> mCurrentPixelIndex;

	SceneObject* mTestSceneObject = nullptr;

	Scene* mScene;

	int mMinDepth;
	int mMaxDepth;
};

