#pragma once

#include "tinyxml2.h"

class Camera;
class Scene;

class SceneLoader
{
public:
    // 先读取一次 XML，再分别加载相机和场景对象；失败返回 false。
    bool LoadFile(const char* filename);

    // 填充传入的相机；XML 中的 Fov 使用角度，W、H 为视口大小。
    bool LoadCamera(Camera& camera, int W, int H) const;

    // 向现有 Scene 追加对象和图元，Rotation 使用角度。
    // 对象通过 Scene 的工厂创建；中途失败时，已创建的对象仍由 Scene 管理。
    bool LoadSceneObjects(Scene& scene) const;

private:
    tinyxml2::XMLDocument mDocument;
};
