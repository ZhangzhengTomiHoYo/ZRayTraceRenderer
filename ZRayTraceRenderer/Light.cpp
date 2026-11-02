#include "Light.h"

Color SpotLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const
{
	pdf = 1.0f;
	sourcePos = mPosition;

	// 距离衰减 k1
	float R = glm::length(p - mPosition);
	float k1 = 1.0f / glm::max(1e-5f,(mAttenuations.x * R * R + mAttenuations.y * R + mAttenuations.z));

	// 角度衰减 k2
	Vector3f L = glm::normalize(p - mPosition);
	float cosTheta = glm::dot(L, mDirection);

	float k2 = (cosTheta - mCosOuterAngle) / (mCosInnerAngle - mCosOuterAngle);

	return mIntensity * k1 * glm::clamp(k2, 0.0f, 1.0f);
}

Color PointLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const
{
	pdf = 1.0f;
	sourcePos = mPosition;
	float R = glm::length(p - mPosition);
	float atten = mAttenuations.x * R * R + mAttenuations.y * R + mAttenuations.z;
	if (atten < 1e-6)
	{
		// TODO
		atten = 1e-5f;
	}
	float attenuation = 1.0f / atten;
	return mIntensity * attenuation;
}

Color DirectionalLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const
{
	pdf = 1.0f;
	sourcePos = p - mDirection * 1000.0f;  // 假设光源在无限远处
	return mRadiance;

}

Color AreaLight::GetRadiance(const Vector3f& p, Vector3f& sourcePos, float& pdf) const
{
	Vector3f A;
	Vector3f normalA;
	float pdf_A;
	m_pSceneObject->Sample(A, normalA, pdf_A);

	sourcePos = A;

	Vector3f d = p - A;
	float R = glm::length(d);
	d = d / glm::max(1e-5f, R);

	float cosThetaA = glm::dot(d, normalA);
	if (cosThetaA < 1e-5f)
	{
		// < 0 打到了面光源的背面
		pdf = 0.0f;
		return Color(0.0f);
	}

	pdf = R * R * pdf_A / cosThetaA;

	return m_pSceneObject->GetEmissive();
}
