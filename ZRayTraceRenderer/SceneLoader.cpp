#include "SceneLoader.h"
#include "Scene.h"
#include "Sphere.h"
#include "Disk.h"
#include "Triangle.h"
#include "tinyxml2.h"

#include <cmath>
#include <sstream>
#include <string>

// SceneLoader 完全由 GPT 6.1 Sol Ultra 编码

namespace
{
    const char* GetChildText(const tinyxml2::XMLElement* parent, const char* name)
    {
        const tinyxml2::XMLElement* child = parent->FirstChildElement(name);
        return child ? child->GetText() : nullptr;
    }

    bool ParseVector3f(const char* text, Vector3f& value)
    {
        if (!text)
            return false;

        std::istringstream stream(text);
        char comma1, comma2;
        if (!(stream >> value.x >> comma1 >> value.y >> comma2 >> value.z)
            || comma1 != ',' || comma2 != ',')
            return false;

        stream >> std::ws;
        return stream.eof() && std::isfinite(value.x)
            && std::isfinite(value.y) && std::isfinite(value.z);
    }

    float GetChildFloat(const tinyxml2::XMLElement* parent, const char* name, float defaultValue)
    {
        const tinyxml2::XMLElement* child = parent->FirstChildElement(name);
        float value = defaultValue;
        if (!child || child->QueryFloatText(&value) != tinyxml2::XML_SUCCESS)
            return defaultValue;
        return value;
    }
}

bool SceneLoader::LoadFile(const char* filename)
{
    mDocument.Clear();
    if (!filename || !*filename)
        return false;

    if (mDocument.LoadFile(filename) != tinyxml2::XML_SUCCESS)
        return false;

    return mDocument.FirstChildElement("Scene") != nullptr;
}

bool SceneLoader::LoadCamera(Camera& camera, int W, int H) const
{
    if (W <= 0 || H <= 0)
        return false;

    const tinyxml2::XMLElement* root = mDocument.FirstChildElement("Scene");
    if (!root)
        return false;

    const tinyxml2::XMLElement* cameraElem = root->FirstChildElement("Camera");
    if (!cameraElem)
        return false;

    Vector3f position, target, up;
    if (!ParseVector3f(GetChildText(cameraElem, "Position"), position)
        || !ParseVector3f(GetChildText(cameraElem, "Target"), target)
        || !ParseVector3f(GetChildText(cameraElem, "Up"), up))
        return false;

    float nearZ = GetChildFloat(cameraElem, "NearZ", 0.1f);
    float farZ = GetChildFloat(cameraElem, "FarZ", 1000.0f);
    float fovDeg = GetChildFloat(cameraElem, "Fov", 45.0f);
    if (!std::isfinite(nearZ) || !std::isfinite(farZ) || !std::isfinite(fovDeg)
        || nearZ <= 0.0f || farZ <= nearZ || fovDeg <= 0.0f || fovDeg >= 180.0f
        || glm::length(target - position) == 0.0f
        || glm::length(glm::cross(up, target - position)) == 0.0f)
        return false;

    camera.Init(position, target, up, glm::radians(fovDeg), nearZ, farZ, W, H);
    return true;
}

bool SceneLoader::LoadSceneObjects(Scene& scene) const
{
    const tinyxml2::XMLElement* root = mDocument.FirstChildElement("Scene");
    if (!root)
        return false;

    const tinyxml2::XMLElement* objectsElem = root->FirstChildElement("SceneObjects");
    if (objectsElem)
    {
        for (const tinyxml2::XMLElement* objectElem = objectsElem->FirstChildElement("SceneObject");
            objectElem; objectElem = objectElem->NextSiblingElement("SceneObject"))
        {
            const tinyxml2::XMLElement* transformElem = objectElem->FirstChildElement("Transform");
            if (!transformElem)
                continue;

            Vector3f objectPosition, rotation;
            if (!ParseVector3f(GetChildText(transformElem, "Position"), objectPosition)
                || !ParseVector3f(GetChildText(transformElem, "Rotation"), rotation))
                return false;

            float scale = GetChildFloat(transformElem, "Scale", 1.0f);
            if (!std::isfinite(scale) || scale == 0.0f)
                return false;

            SceneObject* sceneObject = scene.CreateSceneObject(objectPosition, glm::radians(rotation), scale);
            const tinyxml2::XMLElement* primitivesElem = objectElem->FirstChildElement("Primitives");
            if (!primitivesElem)
                continue;

            // 按 XML 中的顺序创建各个图元。
            for (const tinyxml2::XMLElement* primitiveElem = primitivesElem->FirstChildElement();
                primitiveElem; primitiveElem = primitiveElem->NextSiblingElement())
            {
                const std::string name = primitiveElem->Name();
                if (name == "Sphere" || name == "Disk")
                {
                    float radius = GetChildFloat(primitiveElem, "Radius", 1.0f);
                    if (!std::isfinite(radius) || radius <= 0.0f)
                        return false;

                    if (name == "Sphere")
                        sceneObject->CreatePrimitive<Sphere>(radius);
                    else
                        sceneObject->CreatePrimitive<Disk>(radius);
                }
                else if (name == "Triangle")
                {
                    const tinyxml2::XMLElement* v0Elem = primitiveElem->FirstChildElement("Vertex");
                    const tinyxml2::XMLElement* v1Elem = v0Elem ? v0Elem->NextSiblingElement("Vertex") : nullptr;
                    const tinyxml2::XMLElement* v2Elem = v1Elem ? v1Elem->NextSiblingElement("Vertex") : nullptr;
                    Vector3f v0, v1, v2;
                    if (!v0Elem || !v1Elem || !v2Elem
                        || !ParseVector3f(v0Elem->GetText(), v0)
                        || !ParseVector3f(v1Elem->GetText(), v1)
                        || !ParseVector3f(v2Elem->GetText(), v2))
                        return false;

                    sceneObject->CreatePrimitive<Triangle>(v0, v1, v2);
                }
            }
        }
    }

    return true;
}
