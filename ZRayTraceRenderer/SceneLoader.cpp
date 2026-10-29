#include "SceneLoader.h"
#include "Scene.h"
#include "Sphere.h"
#include "Disk.h"
#include "Triangle.h"
#include "Light.h"
#include "Material.h"
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

bool SceneLoader::LoadMaterials(Scene& scene) const
{
    const tinyxml2::XMLElement* root = mDocument.FirstChildElement("Scene");
    if (!root)
        return false;

    const tinyxml2::XMLElement* materialsElem = root->FirstChildElement("Materials");
    if (!materialsElem)
        return true;

    const auto& materials = scene.GetMaterials();
    for (const tinyxml2::XMLElement* materialElem = materialsElem->FirstChildElement("Material");
        materialElem; materialElem = materialElem->NextSiblingElement("Material"))
    {
        const char* name = GetChildText(materialElem, "Name");
        const char* type = GetChildText(materialElem, "Type");
        if (!name || !*name || !type)
            return false;

        if (materials.find(name) != materials.end())
            return false;

        const std::string materialType = type;
        if (materialType == "Lambert")
        {
            Color albedo;
            if (!ParseVector3f(GetChildText(materialElem, "Albedo"), albedo))
                return false;

            scene.CreateMatrial<LambertMaterial>(name, albedo);
        }
        else if (materialType == "ConductorSpecular")
        {
            Color eta, absorptionCoef, reflectionColor;
            if (!ParseVector3f(GetChildText(materialElem, "Eta"), eta)
                || !ParseVector3f(GetChildText(materialElem, "AbsorptionCoef"), absorptionCoef)
                || !ParseVector3f(GetChildText(materialElem, "ReflectionColor"), reflectionColor))
                return false;

            scene.CreateMatrial<ConductorSpecularMaterial>(name, eta, absorptionCoef, reflectionColor);
        }
        else
        {
            return false;
        }
    }

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
        const auto& materials = scene.GetMaterials();
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

            Material* material = nullptr;
            const tinyxml2::XMLElement* materialElem = objectElem->FirstChildElement("Material");
            if (materialElem)
            {
                const char* materialName = materialElem->GetText();
                if (!materialName || !*materialName)
                    return false;

                auto it = materials.find(materialName);
                if (it == materials.end() || !it->second)
                    return false;
                material = it->second;
            }

            SceneObject* sceneObject = scene.CreateSceneObject(objectPosition, glm::radians(rotation), scale);
            if (material)
                sceneObject->SetMaterial(material);

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

bool SceneLoader::LoadLights(Scene& scene) const
{
    const tinyxml2::XMLElement* root = mDocument.FirstChildElement("Scene");
    if (!root)
        return false;

    const tinyxml2::XMLElement* lightsElem = root->FirstChildElement("Lights");
    if (!lightsElem)
        return true;

    for (const tinyxml2::XMLElement* lightElem = lightsElem->FirstChildElement();
        lightElem; lightElem = lightElem->NextSiblingElement())
    {
        const std::string name = lightElem->Name();
        if (name == "DirectionalLight")
        {
            Vector3f direction;
            Color radiance;
            if (!ParseVector3f(GetChildText(lightElem, "Direction"), direction)
                || !ParseVector3f(GetChildText(lightElem, "Radiance"), radiance)
                || glm::length(direction) == 0.0f)
                return false;

            scene.CreateLight<DirectionalLight>(direction, radiance);
        }
        else if (name == "PointLight" || name == "SpotLight")
        {
            Vector3f position, attenuations;
            Color intensity;
            if (!ParseVector3f(GetChildText(lightElem, "Position"), position)
                || !ParseVector3f(GetChildText(lightElem, "Intensity"), intensity)
                || !ParseVector3f(GetChildText(lightElem, "Attenuations"), attenuations))
                return false;

            if (attenuations.x < 0.0f || attenuations.y < 0.0f || attenuations.z < 0.0f
                || glm::length(attenuations) == 0.0f)
                return false;

            if (name == "PointLight")
            {
                scene.CreateLight<PointLight>(position, intensity, attenuations);
            }
            else
            {
                Vector3f direction;
                if (!ParseVector3f(GetChildText(lightElem, "Direction"), direction)
                    || glm::length(direction) == 0.0f)
                    return false;

                float innerAngle = GetChildFloat(lightElem, "InnerAngle", 0.0f);
                float outerAngle = GetChildFloat(lightElem, "OuterAngle", 0.0f);
                if (!std::isfinite(innerAngle) || !std::isfinite(outerAngle)
                    || innerAngle < 0.0f || innerAngle >= outerAngle || outerAngle > 180.0f)
                    return false;

                scene.CreateLight<SpotLight>(position, direction, intensity,
                    glm::radians(innerAngle), glm::radians(outerAngle), attenuations);
            }
        }
    }

    return true;
}
