#pragma once

#include "tinyxml2.h"

class Camera;
class Scene;

class SceneLoader
{
public:
    // 字段读取沿用老师的写法：向量解析失败为零，浮点字段缺失使用默认值。
    // 先读取一次 XML，再分别加载相机、材质、场景对象和光源。
    // 文件或 Scene 根节点无效时返回 false。
    bool LoadFile(const char* filename);

    // 填充传入的相机；Fov 使用角度，W、H 为视口大小。
    // 缺少 Camera 或视口大小无效时返回 false。
    bool LoadCamera(Camera& camera, int W, int H) const;

    // 按名称创建 Lambert、ConductorSpecular 或 DielectricSpecular 材质；必须在 LoadSceneObjects 之前调用。
    // 未知类型或缺少名称、类型的材质跳过。
    bool LoadMaterials(Scene& scene) const;

    // 向现有 Scene 追加对象和图元，Rotation 使用角度。
    // Material 按名称查找；找不到时不设置材质。Emissive 设置对象自发光。
    // 缺少 Transform 的对象、缺少三个 Vertex 的三角形跳过。
    // 图元按 Sphere、Disk、Triangle 分组加载；自发光对象注册为面光源。
    bool LoadSceneObjects(Scene& scene) const;

    // 从 Lights 节点追加 DirectionalLight、PointLight 和 SpotLight。
    // 聚光灯角度使用度；没有 Lights 节点时成功，不创建光源。
    // 按 DirectionalLight、PointLight、SpotLight 分组加载。
    bool LoadLights(Scene& scene) const;

private:
    mutable tinyxml2::XMLDocument mDocument;
};
