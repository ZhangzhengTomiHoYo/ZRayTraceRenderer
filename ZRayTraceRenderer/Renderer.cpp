#include <thread>
#include <chrono>
#include <iostream>
#include <vector>
#include <MiniFB.h>

#include "Renderer.h"
#include "Common.h"
#include "Material.h"

Renderer::Renderer() {} ;

Renderer::Renderer(const char* filename, int width, int height) : mViewportWidth(width), mViewportHeight(height) 
{
    mScene = new Scene();
    bool result = mScene->LoadFromXML(filename, width, height);
    if (!result)
    {
        throw std::runtime_error("Failed to load scene data!");
    }
};

Renderer::~Renderer() 
{
    if (mScene)
    {
        delete mScene;
    }
};

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

Color Renderer::GetIrridance(Ray& ray)
{
    Intersection isect;
    if (!mScene->Intersect(ray, isect))
    {
        return Color(0, 0, 0);
    }

    Color E(0, 0, 0);

    // E(p)
    for (Light* pLight : mScene->GetLights())
    {
        Vector3f sourcePos;
        Color L = pLight->GetRadiance(isect.position, sourcePos);

        // 检测交点到光源有没有遮挡
        // 求 ShadowRay
        Ray shadowRay;
        shadowRay.o = isect.position;
        shadowRay.d = glm::normalize(sourcePos - isect.position);
        shadowRay.mint = 1e-4f;
        shadowRay.maxt = glm::length(sourcePos - isect.position);

        Intersection shadowIsect;
        if (mScene->Intersect(shadowRay, shadowIsect))
        {
            continue;
        }

        float cosTheta = glm::dot(isect.normal, shadowRay.d);

        E += L * glm::max(cosTheta, 0.0f);
    }

    return E;
}

Color Renderer::GetRadiance(Ray& ray)
{
    Intersection isect;
    SceneObject* pSceneObject = mScene->Intersect(ray, isect);
    if (pSceneObject == nullptr)
    {
        return Color(0, 0, 0);
    }
    Material* pMaterial = pSceneObject->GetMaterial();
    Matrix3x3 localToWorld = makeCoordinateSystem(isect.normal);
    Matrix3x3 worldToLocal = glm::inverse(localToWorld);
    Vector3f wo = worldToLocal * (-ray.d);

    Color Lo(0, 0, 0);

    // E(p)
    for (Light* pLight : mScene->GetLights())
    {
        Vector3f sourcePos;
        Color Li = pLight->GetRadiance(isect.position, sourcePos);

        // 检测交点到光源有没有遮挡
        // 求 ShadowRay
        Ray shadowRay;
        shadowRay.o = isect.position;
        shadowRay.d = glm::normalize(sourcePos - isect.position);
        shadowRay.mint = 1e-4f;
        shadowRay.maxt = glm::length(sourcePos - isect.position);

        Intersection shadowIsect;
        if (mScene->Intersect(shadowRay, shadowIsect))
        {
            continue;
        }
        
        float cosTheta = glm::dot(isect.normal, shadowRay.d);
        Vector3f wi = worldToLocal * shadowRay.d;
        Color brdf = pMaterial->BRDF(wo, wi);
        Lo += brdf * Li * glm::max(cosTheta, 0.0f);
    }

    return Lo;
}

// NOTE: 渲染代码阅读入口
Color Renderer::RenderPixel(int x, int y)
{
    // SSAA
    static const int N = 100;
    Color resultColor{ 0,0,0 };
    for (size_t i = 0; i < N; i++)
    {
        float px = x + glm::linearRand(0.0f, 1.0f);
        float py = y + glm::linearRand(0.0f, 1.0f);

        Color color = RenderSubPixel(px, py);
        resultColor += (color / static_cast<float>(N));
    }

    return resultColor;
}

Color Renderer::RenderSubPixel(float x, float y)
{
    Ray ray = mScene->GetCamera().GetRay(x, y);
    return GetRadiance(ray);
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
