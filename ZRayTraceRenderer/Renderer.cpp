#include <thread>
#include <chrono>
#include <iostream>
#include <vector>

#include "Renderer.h"
#include <MiniFB.h>
#include "Common.h"

Renderer::Renderer() {} ;
Renderer::Renderer(int width, int height) : mViewportWidth(width), mViewportHeight(height) {};
Renderer::~Renderer() {};

void Renderer::Run()
{
    struct mfb_window* window = mfb_open_ex("ZRayTraceRenderer", mViewportWidth, mViewportHeight, MFB_WF_RESIZABLE);
    if (window == NULL)
        return;

    // 窗口/视口 上每个像素点的颜色，以32位整数表示，格式为0xAARRGGBB
    mBuffer = static_cast<uint32_t*>(malloc(mViewportWidth * mViewportHeight * 4));

    int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> renderThreads(numThreads);
    for (int i = 0; i < numThreads; i++)
    {
        renderThreads[i] = std::thread(&Renderer::RunRenderThread, this);
        renderThreads[i].detach();
    }

    mfb_update_state state;
    do {
        state = mfb_update_ex(window, mBuffer, mViewportWidth, mViewportHeight);

        if (state != MFB_STATE_OK)
            break;

    } while (mfb_wait_sync(window));

    free(mBuffer);
    mBuffer = NULL;
    window = NULL;

}

Color Renderer::RenderPixel(int x, int y)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    Color color;
    color.r = static_cast<float>(x) / mViewportWidth;
    color.g = static_cast<float>(y) / mViewportHeight;
    color.b = 0.0f;
    return color;
}

// 渲染线程的入口函数
void Renderer::RunRenderThread()
{
    // 读取当前屏幕的下一个像素
    while (true)
    {
        int pixelIndex = mCurrentPixelIndex.fetch_add(1);

        if (pixelIndex >= mViewportHeight * mViewportWidth)
            break;

        int x = pixelIndex % mViewportWidth;
        int y = pixelIndex / mViewportWidth;

        // buffer 32位 为 一个元素
        // MiniFB窗口库 布局为 ARGB
        // r 最大为 255 从低位往高位占8位 从低往高第8位 现在要变成 从低往高第24位 所以左移16位 
        // 使用或运算 只要有一个为1就为1
        Color color = RenderPixel(x, y);
        uint32_t r = glm::clamp(static_cast<uint32_t>(std::round(color.r * 255.0f)), 0u, 255u);
        uint32_t g = glm::clamp(static_cast<uint32_t>(std::round(color.g * 255.0f)), 0u, 255u);
        uint32_t b = glm::clamp(static_cast<uint32_t>(std::round(color.b * 255.0f)), 0u, 255u);
        mBuffer[y * mViewportWidth + x] = (r << 16) | (g << 8) | (b);
    }
}
