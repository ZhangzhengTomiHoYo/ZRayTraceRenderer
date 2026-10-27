#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using Vector3f = glm::vec3;
using Vector4f = glm::vec4;
using Matrix4x4 = glm::mat4;
using Color = glm::vec3;
const float PI = glm::pi<float>();

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