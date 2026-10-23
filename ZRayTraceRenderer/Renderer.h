#pragma once
#include <atomic>
#include "Common.h"
#include "Camera.h"

class Renderer
{
public:
	Renderer();
	Renderer(int width, int height);
	~Renderer();
	void Run();

private:
	Color RenderPixel(int x, int y);
	void RunRenderThread();
	int mViewportWidth;
	int mViewportHeight;
	uint32_t* mBuffer;
	std::atomic<int> mCurrentPixelIndex;

	Camera mCamera;
};

