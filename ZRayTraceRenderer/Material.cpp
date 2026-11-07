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

Vector3f DielectrricSpecularMaterial::SampleWi(const Vector3f& wo, float& pdf) const
{
    Vector3f wi(-wo.x, -wo.y, wo.z);
    pdf = 1.0f;

    return wi;
}

float DielectrricSpecularMaterial::Fresnel(float eta_i, float eta_t, float cos_i, float cos_t)
{
    float r1 = (eta_t * cos_i - eta_i * cos_t) / (eta_t * cos_i + eta_i * cos_t);
    float r2 = (eta_i * cos_i - eta_t * cos_t) / (eta_i * cos_i + eta_t * cos_t);
    return 0.5f * (r1 * r1 + r2 * r2);
}

Vector3f LambertMaterial::SampleWi(const Vector3f& wo, float& pdf) const
{
    float e1 = Random01();
    float e2 = Random01();

    float cosTheta = sqrtf(1.0f - e1);
    float sinTheta = sqrtf(e1);
    float phi = 2.0f * PI * e2;

    Vector3f wi;
    wi.x = sinTheta * cos(phi);
    wi.y = sinTheta * sin(phi);
    wi.z = cosTheta;

    pdf = INV_PI * cosTheta;

    return wi;
}

Vector3f ConductorSpecularMaterial::SampleWi(const Vector3f& wo, float& pdf) const
{
    Vector3f wi(-wo.x, -wo.y, wo.z);
    pdf = 1.0f;

    return wi;
}

Color OrenNayarMaterial::BRDF(const Vector3f& wo, const Vector3f& wi) const
{
    // 不计算物体背面
    if (wo.z <= 0.0f || wi.z <= 0.0f)
    {
        return Color(0);
    }
    static const float epsilon = 1e-5f;

    float sigma2 = mSigma * mSigma;

    float A = 1.0f - (sigma2 / 2.0f * (sigma2 + 0.33f));
    float B = 0.45f * sigma2 / (sigma2 + 0.09f);

    float cos_theta_i = wi.z;
    float sin_theta_i = sqrtf(glm::max(epsilon, 1.0f - cos_theta_i * cos_theta_i));
    float cos_phi_i = wi.x / glm::max(epsilon, sin_theta_i);
    float sin_phi_i = wi.y / glm::max(epsilon, sin_theta_i);
    float tan_theta_i = sin_theta_i / glm::max(epsilon, cos_theta_i);

    float cos_theta_o = wo.z;
    float sin_theta_o = sqrtf(glm::max(epsilon, 1.0f - cos_theta_o * cos_theta_o));
    float cos_phi_o = wo.x / glm::max(epsilon, sin_theta_o);
    float sin_phi_o = wi.y / glm::max(epsilon, sin_theta_o);
    float tan_theta_o = sin_theta_o / glm::max(epsilon, cos_theta_o);

    // alpha max(theta_i, theta_o)
    float sin_alpha = glm::max(sin_theta_i, sin_theta_o);
    // beta min(theta_i, theta_o)
    float tan_beta = glm::min(tan_theta_i, tan_theta_o);

    // cos(phi_i - phi_o)
    float cos_delta_phi = cos_phi_i * cos_phi_o + sin_phi_i * sin_phi_o;

    return mAlbedo * INV_PI * (A + B * glm::max(0.0f, cos_delta_phi) * sin_alpha * tan_beta);
}

Vector3f OrenNayarMaterial::SampleWi(const Vector3f& wo, float& pdf) const
{
    // ON材质的BRDF十分复杂
    // pdf按 cos_theta_i 进行重要性采样
    // 和Lambert材质的重要性采样一样

    float e1 = Random01();
    float e2 = Random01();

    float cosTheta = sqrtf(1.0f - e1);
    float sinTheta = sqrtf(e1);
    float phi = 2.0f * PI * e2;

    Vector3f wi;
    wi.x = sinTheta * cos(phi);
    wi.y = sinTheta * sin(phi);
    wi.z = cosTheta;

    pdf = INV_PI * cosTheta;

    return wi;
}

Color TorranceSparrowMaterial::BRDF(const Vector3f& wo, const Vector3f& wi) const
{
    static const float epsilon = 1e-5f;
    if (wo.z <= epsilon || wi.z <= epsilon)
        return Color(0);

    Vector3f wh = glm::normalize(wi + wo);

    // D:
    float D = (mExponent + 2) * std::pow(wh.z, mExponent) / (2 * PI);

    // G:
    float deno = std::max(epsilon, glm::dot(wo, wh));
    float G = std::min(1.0f, std::min(2 * wh.z * wo.z / deno, 2 * wh.z * wi.z / deno));

    // F:
    float cosTheta = std::max(1e-5f, glm::dot(wh, wi));
    Color k = mAbsorptionCoef;
    Color r1 = ((mEta * mEta + k * k) * cosTheta * cosTheta - 2.0f * mEta * cosTheta + 1.0f)
        / ((mEta * mEta + k * k) * cosTheta * cosTheta + 2.0f * mEta * cosTheta + 1.0f);

    Color r2 = (mEta * mEta + k * k - 2.0f * mEta * cosTheta + cosTheta * cosTheta)
        / (mEta * mEta + k * k + 2.0f * mEta * cosTheta + cosTheta * cosTheta);
    Color F = (r1 + r2) * 0.5f;

    return mReflectionColor * D * F * G / (4 * wo.z * wi.z);
}

Vector3f TorranceSparrowMaterial::SampleWi(const Vector3f& wo, float& pdf) const
{
    float e1 = Random01();
    float e2 = Random01();

    float cos_theta_h = std::pow(e1, 1.0f / (mExponent + 1));
    float sin_theta_h = std::sqrt(std::max(0.0f, 1 - cos_theta_h * cos_theta_h));
    float phi_h = 2 * PI * e2;

    Vector3f wh;
    wh.x = sin_theta_h * cos(phi_h);
    wh.y = sin_theta_h * sin(phi_h);
    wh.z = cos_theta_h;

    Vector3f wi = 2 * glm::dot(wo, wh) * wh - wo;

    float pdf_wh = (mExponent + 1) * std::pow(cos_theta_h, mExponent) / (2 * PI);

    pdf = pdf_wh / std::max(1e-5f, 4 * glm::dot(wo, wh));

    return wi;
}
