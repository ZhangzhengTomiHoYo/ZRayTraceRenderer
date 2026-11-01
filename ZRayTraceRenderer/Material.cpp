#include "Material.h"

Color DielectrricSpecularMaterial::BRDF(const Vector3f& wo, const Vector3f& wi) const
{
    float cos_i = fabs(wi.z);
    float eta_i, eta_t;
    if (wi.z > 0)
    {
        eta_i = 1.0f;
        eta_t = mEta;
    }
    else
    {
        eta_i = mEta;
        eta_t = 1.0f;
    }
    // 绝缘体求BRDF 虽然不会用到wt本身  但是需要wt去求theta_t
    Vector3f wt;
    float Fr = 1.0f;
    if (ComputeRefractVector(wi, eta_i, eta_t, wt))
    {
        float cos_t = fabs(wt.z);
        Fr = Fresnel(eta_i, eta_t, cos_i, cos_t);
    }

    return Color(Fr) / std::max(cos_i, 1e-4f);
}

Color DielectrricSpecularMaterial::BTDF(const Vector3f& wt, const Vector3f& wi) const
{
    float cos_i = fabs(wi.z);
    float eta_i, eta_t;
    if (wi.z > 0)
    {
        eta_i = 1.0f;
        eta_t = mEta;
    }
    else
    {
        eta_i = mEta;
        eta_t = 1.0f;
    }
    float cos_t = fabs(wt.z);

    float Fr = Fresnel(eta_i, eta_t, cos_i, cos_t);

    return  mTransmissionColor * (1.0f - Fr) * eta_t * eta_t / (eta_i * eta_i * std::max(1e-4f, cos_i));
}

bool DielectrricSpecularMaterial::SampleWt(const Vector3f& wi, Vector3f& wt) const
{
    float cos_i = fabs(wi.z);
    float eta_i, eta_t;
    if (wi.z > 0)
    {
        eta_i = 1.0f;
        eta_t = mEta;
    }
    else
    {
        eta_i = mEta;
        eta_t = 1.0f;
    }

    return ComputeRefractVector(wi, eta_i, eta_t, wt);
}

float DielectrricSpecularMaterial::Fresnel(float eta_i, float eta_t, float cos_i, float cos_t)
{
    float r1 = (eta_t * cos_i - eta_i * cos_t) / (eta_t * cos_i + eta_i * cos_t);
    float r2 = (eta_i * cos_i - eta_t * cos_t) / (eta_i * cos_i + eta_t * cos_t);
    return 0.5f * (r1 * r1 + r2 * r2);
}
