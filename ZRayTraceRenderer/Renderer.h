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
	Renderer(int width, int height);
	~Renderer();
	void Run();

private:
	Color RenderPixel(int x, int y);
	Color RenderSubPixel(float x, float y);
	void RunRenderThread();
	int mViewportWidth;
	int mViewportHeight;
	uint32_t* mBuffer;
	std::atomic<int> mCurrentPixelIndex;

	//Camera mCamera;

	//std::vector<Primitive*> mPrimitives;
	SceneObject* mTestSceneObject = nullptr;

	Scene* mScene;
};

