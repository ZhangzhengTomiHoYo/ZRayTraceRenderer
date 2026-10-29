#pragma once

#include "tinyxml2.h"

class Camera;
class Scene;

class SceneLoader
{
public:
    // 先读取一次 XML，再分别加载相机、材质、场景对象和光源；失败返回 false。
    bool LoadFile(const char* filename);

    // 填充传入的相机；XML 中的 Fov 使用角度，W、H 为视口大小。
    bool LoadCamera(Camera& camera, int W, int H) const;

    // 按名称创建 Lambert 材质；必须在 LoadSceneObjects 之前调用。
    // 没有 Materials 节点时成功；重名、未知类型或无效数据返回 false。
    // 中途失败时，已创建的材质仍由 Scene 管理。
    bool LoadMaterials(Scene& scene) const;

    // 向现有 Scene 追加对象和图元，Rotation 使用角度。
    // Material 节点按名称引用已加载的材质；引用不存在时返回 false。
    // 对象通过 Scene 的工厂创建；中途失败时，已创建的对象仍由 Scene 管理。
    bool LoadSceneObjects(Scene& scene) const;

    // 从 Lights 节点追加 DirectionalLight、PointLight 和 SpotLight。
    // 聚光灯角度使用度；没有 Lights 节点时成功，不创建光源。
    // 中途失败时，已创建的光源仍保存在 Scene 中。
    bool LoadLights(Scene& scene) const;

private:
    tinyxml2::XMLDocument mDocument;
};
