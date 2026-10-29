#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

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