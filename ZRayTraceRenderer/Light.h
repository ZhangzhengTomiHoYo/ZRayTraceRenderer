#pragma once
#include "Ray.h"

class Light
{
public:
	// 对于一点，求它的L(p)
	virtual Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const = 0;
};

// 平行光
class DirectionalLight : public Light
{
public:
	DirectionalLight(const Vector3f& direction, const Color& radiance) : mDirection(direction), mRadiance(radiance) {}

	Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const override;

private:
	Vector3f mDirection;     // 光线方向 单位向量
	Color	 mRadiance;      // 光照强度
};

// 点光源
class PointLight : public Light
{
public:
	PointLight(const Vector3f& position, const Color& intensity, const Vector3f& attenuations) : mPosition(position), mIntensity(intensity), mAttenuations(attenuations){}

	Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const override;

private:
	Vector3f mPosition;       // 光源位置
	Color    mIntensity;      // 光源强度
	Vector3f mAttenuations;   // 衰减系数
};

// 聚光灯
class SpotLight : public Light
{
public:
	SpotLight(const Vector3f& position, const Vector3f& direction, const Color& intensity, float innerAngle, float outerAngle, const Vector3f& attenuations) : 
		mPosition(position), mDirection(glm::normalize(direction)), mIntensity(intensity), mCosInnerAngle(std::cos(innerAngle)), mCosOuterAngle(std::cos(outerAngle)), mAttenuations(attenuations) {}

	virtual Color GetRadiance(const Vector3f& p, Vector3f& sourcePos) const override;
private:
	Vector3f mDirection;  // 光线方向 单位向量
	Vector3f mPosition;   // 光源位置
	Color mIntensity;     // 光源强度

	float mCosInnerAngle;
	float mCosOuterAngle;

	Vector3f mAttenuations;
};