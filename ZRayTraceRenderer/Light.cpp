#include "Light.h"

Color SpotLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos) const
{
	sourcePos = mPosition;

	// 距离衰减 k1
	float R = glm::length(p - mPosition);
	float k1 = 1.0f / mAttenuations.x * R * R + mAttenuations.y * R + mAttenuations.z;

	// 角度衰减 k2
	Vector3f L = glm::normalize(p - mPosition);
	float cosTheta = glm::dot(L, mDirection);

	float k2 = (cosTheta - mCosOuterAngle) / (mCosInnerAngle - mCosOuterAngle);

	return mIntensity * k1 * glm::clamp(k2, 0.0f, 1.0f);
}

Color PointLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos) const
{
	sourcePos = mPosition;
	float R = glm::length(p - mPosition);
	float atten = mAttenuations.x * R * R + mAttenuations.y * R + mAttenuations.z;
	if (atten < 1e-6)
	{
		// TODO
	}
	float attenuation = 1.0f / atten;
	return mIntensity * attenuation;
}

Color DirectionalLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos) const
{
	sourcePos = p - mDirection * 1000.0f;  // 假设光源在无限远处
	return mRadiance;

}
