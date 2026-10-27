#include "Triangle.h"

Triangle::Triangle(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, const Matrix4x4 worldMatrix)
{
	mVertices[0] = Vector3f(worldMatrix * Vector4f(v0, 1.0));
	mVertices[1] = Vector3f(worldMatrix * Vector4f(v1, 1.0));
	mVertices[2] = Vector3f(worldMatrix * Vector4f(v2, 1.0));

	Vector3f edge1 = mVertices[1] - mVertices[0];
	Vector3f edge2 = mVertices[2] - mVertices[1];
	mNormal = glm::normalize(glm::cross(edge1, edge2));


}

bool Triangle::Intersect(Ray& ray, Intersection& isect) const
{
	// 球、圆盘 转到模型空间做求交 会带来便利
	// 但是三角形转到模型空间没有便利 所以直接在世界空间做

	// 下方公式 为 使用克莱姆法则求解线性方程组所得
	// 并合并了重复的计算 以减少计算量

	Vector3f p0 = mVertices[0];
	Vector3f p1 = mVertices[1];
	Vector3f p2 = mVertices[2];

	Vector3f e1 = p1 - p0;
	Vector3f e2 = p2 - p0;
	Vector3f s = ray.o - p0;

	Vector3f s1 = glm::cross(ray.d, e2);
	Vector3f s2 = glm::cross(s, e1);

	float det = glm::dot(s1, e1);
	if (fabs(det) < 1e-6)
	{
		return false;
	}
	float invDet = 1.0f / det;
	float b1 = glm::dot(s1, s) * invDet;
	float b2 = glm::dot(s2, ray.d) * invDet;
	float t = glm::dot(s2, e2) * invDet;

	if (t < ray.mint || t > ray.maxt)
	{
		// 不在线段上
		return false;
	}

	float b0 = 1.0f - b1 - b2;
	if (b1 < 0.0f || b2 < 0.0f || b0 < 0.0f)
	{
		// 交点在三角形外面
		return false;
	}

	isect.position = ray.o + t * ray.d;
	isect.normal = mNormal;
	isect.t = t;

	return true;
}
