#include "Material.h"

Color DielectrricSpecularMaterial::BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const
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

Vector3f DielectrricSpecularMaterial::SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const
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

Color LambertMaterial::BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const
{
    if (wo.z <= 0 || wi.z <= 0)
    {
        return Color(0);
    }
    Vector3f albedo = mAlbedo;
    if (m_pAlbedoMap != nullptr)
    {
        albedo *= m_pAlbedoMap->Sample<Color>(uv);
    }
    return albedo * INV_PI;
}

Vector3f LambertMaterial::SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const
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

Vector3f ConductorSpecularMaterial::SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const
{
    Vector3f wi(-wo.x, -wo.y, wo.z);
    pdf = 1.0f;

    return wi;
}

Color OrenNayarMaterial::BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const
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

    Vector3f albedo = mAlbedo;
    if (m_pAlbedoMap != nullptr)
    {
        albedo *= m_pAlbedoMap->Sample<Color>(uv);
    }

    return albedo * INV_PI * (A + B * glm::max(0.0f, cos_delta_phi) * sin_alpha * tan_beta);
}

Vector3f OrenNayarMaterial::SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const
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

Color TorranceSparrowMaterial::BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const
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

Vector3f TorranceSparrowMaterial::SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const
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

Color CookTorranceMaterial::BRDF(Vector2f uv, const Vector3f& wo, const Vector3f& wi) const
{
    static const float epsilon = 1e-5f;
    if (wo.z <= epsilon || wi.z <= epsilon)
        return Color(0);

    Color albedo = mAlbedo;
    if (m_pAlbedoMap != nullptr)
        albedo *= m_pAlbedoMap->Sample<Color>(uv);

    float roughness = mRoughness;
    if (m_pRoughnessMap != nullptr)
        roughness *= m_pRoughnessMap->Sample<float>(uv);

    // roughness为0 会导致D项完全变为0
    roughness = glm::clamp(roughness, 0.03f, 1.0f);

    float metallic = mMetallic;
    if (m_pMetallicMap != nullptr)
        metallic *= m_pMetallicMap->Sample<float>(uv);

    Vector3f wh = glm::normalize(wi + wo);

    // D:
    float D = DistributionGGX(roughness, wh);

    // F:
    Color F = FresnelSchlick(metallic, albedo,wo, wh);

    // G:
    float G = GeometrySmith(roughness, wi, wo);

    Color Kd = (Color(1.0f) - F) * (1.0f - metallic);
    return Kd * albedo * INV_PI + D * F * G / (4.0f * wo.z * wi.z);

}

Vector3f CookTorranceMaterial::SampleWi(Vector2f uv, const Vector3f& wo, float& pdf) const
{
    // 注意 使用了uv采样roughness

    float roughness = mRoughness;
    if (m_pRoughnessMap != nullptr)
        roughness *= m_pRoughnessMap->Sample<float>(uv);
    roughness = glm::clamp(roughness, 0.03f, 1.0f);
    float a = roughness * roughness;
    float a2 = a * a;

    Vector3f wi;

    float e1 = Random01();
    float e2 = Random01();

    float metallic = mMetallic;
    if (m_pMetallicMap != nullptr)
        metallic *= m_pMetallicMap->Sample<float>(uv);

    // 混合概率分布
    float weight = 0.5f;
    float pdf1 = 0; 
    float pdf2 = 0;

    if (Random01() < weight)
    {
        // CDF反函数法 算积分得到的结果
        float cos_theta_h = std::sqrtf(std::max(0.0f, (1 - e1) / std::max(1e-5f, e1 * (a2 - 1.0f) + 1.0f)));
        float sin_theta_h = std::sqrtf(std::max(0.0f, 1.0f - cos_theta_h * cos_theta_h));
        float phi_h = 2.0f * PI * e2;


        Vector3f wh;
        wh.x = std::cos(phi_h) * sin_theta_h;
        wh.y = std::sin(phi_h) * sin_theta_h;
        wh.z = cos_theta_h;

        float woDotWh = glm::dot(wo, wh);
        // 有可能小于0 会造成pdf为负
        if (woDotWh <= 0.0f)
        {
            pdf = 0.0f;
            return Vector3f(0.0f, 0.0f, 0.0f);
        }

        wi = 2.0f * woDotWh * wh - wo;
        // 由于引入了roughness  wi有可能采样到平面之下
        if (wi.z <= 0.0f)
        {
            pdf = 0.0f;
            return wi;
        }

        // pdf(wh) = D * cos_theta_h;
        // pdf(wi) = pdf(wh) / 4 * woDotwh
        pdf2 = DistributionGGX(roughness, wh) * cos_theta_h / (4.0f * woDotWh);
        pdf1 = wi.z * INV_PI;
    }
    else
    {
        float e1 = Random01();
        float e2 = Random01();

        float cosTheta = sqrtf(1.0f - e1);
        float sinTheta = sqrtf(e1);
        float phi = 2.0f * PI * e2;

        //Vector3f wi;
        wi.x = sinTheta * cos(phi);
        wi.y = sinTheta * sin(phi);
        wi.z = cosTheta;

        pdf1 = INV_PI * cosTheta;

        Vector3f wh = glm::normalize(wo + wi);
        float woDotWh = glm::dot(wo, wh);
        if (woDotWh > 0 && wh.z > 0)
        {
            // cos_theta_h 在GGX中是对wh采样得到的 需要改成 wh.z
            pdf2 = DistributionGGX(roughness, wh) * wh.z / (4.0f * woDotWh);
        }
    }
    pdf = (1.0f - weight) * pdf1 + weight * pdf2;
    return wi;
}

Color CookTorranceMaterial::FresnelSchlick(float metallic, const Color& albedo, const Vector3f& wo, const Vector3f& wh)
{
    Color F0 = glm::mix(Color(0.04f), albedo, metallic);
    float cos_h_v = glm::clamp(glm::dot(wo, wh), 0.0f, 1.0f);
    return F0 + (Color(1.0f) - F0) * std::pow(1.0f - cos_h_v, 5.0f);
}

float CookTorranceMaterial::DistributionGGX(float roughness, const Vector3f& wh)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = wh.z * wh.z * (a2 - 1.0f) + 1.0f;
    denom = PI * denom * denom;
    return a2 / std::max(denom, 1e-8f);
}

float CookTorranceMaterial::GeometrySmith(float roughness, const Vector3f& wi, const Vector3f& wo)
{
    float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
    return GeometrySchlickGGX(k, std::abs(wi.z)) * GeometrySchlickGGX(k, std::abs(wo.z));
}

float CookTorranceMaterial::GeometrySchlickGGX(float k, float NdotV)
{
    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    return nom / std::max(denom, 1e-5f);
}
