#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <random>

using Vector2f = glm::vec2;
using Vector3f = glm::vec3;
using Vector4f = glm::vec4;
using Matrix4x4 = glm::mat4;
using Matrix3x3 = glm::mat3;
using Color = glm::vec3;
const float PI = glm::pi<float>();
const float INV_PI = 1.0f / PI;

inline Matrix4x4 makeTranslation(const Vector3f& v)
{
	return glm::mat4({
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		v.x, v.y, v.z, 1.0f
	});
}

inline Matrix4x4 makeRotation(const Vector3f& euler)
{
	float cx = cosf(euler.x);
	float sx = sinf(euler.x);
	float cy = cosf(euler.y);
	float sy = sinf(euler.y);
	float cz = cosf(euler.z);
	float sz = sinf(euler.z);

	Matrix4x4 rx({
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, cx, sx, 0.0f,
		0.0f, -sx, cx, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	});

	Matrix4x4 ry({
		cy, 0.0f, -sy, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		sy, 0.0f, cy, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	});

	Matrix4x4 rz({
		cz, sz, 0.0f, 0.0f,
		-sz, cz, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	});

	return rz * ry * rx;
}

// 本项目使用等比例缩放
inline Matrix4x4 makeScale(float s)
{
	return Matrix4x4({
		s, 0, 0, 0,
		0, s, 0, 0,
		0, 0, s, 0,
		0, 0, 0, 1
	});
}

// 世界变换矩阵
inline Matrix4x4 makeWorldTransform(const Vector3f& position, const Vector3f& rotation, float s)
{
	Matrix4x4 T = makeTranslation(position);
	Matrix4x4 R = makeRotation(rotation);
	Matrix4x4 S = makeScale(s);

	return T * R * S;
}

// 建系算法 + 构造新坐标系到世界坐标系的变换矩阵
inline Matrix3x3 makeCoordinateSystem(const Vector3f& w)
{
	Vector3f u(1.0f, 0.0f, 0.0f);

	if (glm::dot(u, w) > 0.99f)
	{
		u = Vector3f(0.0f, 1.0f, 0.0f);
	}

	Vector3f v = glm::cross(w, u);
	u = glm::cross(v, w);

	u = glm::normalize(u);
	v = glm::normalize(v);

	return Matrix3x3(u, v, w); // 列主序
}

inline float Random01()
{
	thread_local std::mt19937 gen(std::random_device{}());
	thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	return dist(gen);
}

inline float Random(float a, float b)
{
	return a + (b - a) * Random01();
}

// 从球坐标系转到笛卡尔坐标系:
inline Vector3f getSphericalCoordinate(float theta, float phi)
{
	return Vector3f(
		sinf(theta) * cosf(phi),
		sinf(theta) * sinf(phi),
		cosf(theta)
	);
}

// 使用折射定律求折射角度wt eta代表材质物理信息
inline bool ComputeRefractVector(const Vector3f& wi, float eta_i, float eta_t, Vector3f& wt)
{	
	// 法向量(0,0,1)
	float cos_i = wi.z;
	float sin_i = glm::max(1e-5f,sqrtf(1.0f - cos_i * cos_i));
	// 折射定律得到
	float sin_t = sin_i * eta_i / eta_t;

	// 全反射
	if (sin_t >= 1.0f)
	{
		// 没有折射光线
		return false;
	}

	float cos_t = glm::max(1e-5f, sqrtf(1.0f - sin_t * sin_t));

	// `cos_t = sqrtf(...)`。开方运算的结果永远是非负的，所以这个 `cos_t` 只代表折射角余弦的 “大小”，本身不带任何方向信息。
	// 而折射光线 `wt` 是要穿过界面、跑到另一侧去的，它的法向分量 `wt.z` 必须和入射光 `wi.z` 符号相反，这个符号得手动补回去
	wt.z = wi.z > 0 ? -cos_t : cos_t;
	// 根据球坐标系公式求 利用投影角phi 相差180度
	wt.x = -wi.x * sin_t / glm::max(1e-5f, sin_i);
	wt.y = -wi.y * sin_t / glm::max(1e-5f, sin_i);

	// 肯定是一个单位向量

	return true;
}

// 在2D圆形均匀采样
inline Vector2f UniformSampleDisk(float R)
{
	float e1 = Random01();
	float e2 = Random01();

	float r = R * sqrtf(e1);
	float theta = 2.0f * PI * e2;

	float x = r * cos(theta);
	float y = r * sin(theta);

	return Vector2f(x, y);
}

// 在单位半球面均匀采样
inline Vector3f UniformSampleHemisphere()
{
	float e1 = Random01();
	float e2 = Random01();

	float cosTheta = e1;
	float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

	float phi = 2.0f * PI * e2;

	float x = sinTheta * sin(phi);
	float y = sinTheta * cos(phi);
	float z = cosTheta;

	return Vector3f(x, y, z);
}

// 均匀采样三角形
inline Vector3f UniformSampleTriangle(const Vector3f& p0, const Vector3f& p1, const Vector3f& p2)
{
	float e1 = Random01();
	float e2 = Random01();

	float sqrt_e1 = sqrtf(e1);
	float u = 1.0f - sqrt_e1;
	float v = e2 * sqrt_e1;

	Vector3f p = (1 - u - v) * p0 + u * p1 + v * p2;
	return p;
}