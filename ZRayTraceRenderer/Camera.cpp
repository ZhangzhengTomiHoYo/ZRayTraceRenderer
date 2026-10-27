#include "Camera.h"


void Camera::Init(const Vector3f& p, const Vector3f& target, const Vector3f& up, float fov, float near, float far, int W, int H)
{
	mPosition = p;

	
	//// 求观察矩阵
	//Vector3f L = glm::normalize(target - p); // 前向向量 观察方向 从相机位置指向目标位置的向量
	//Vector3f R = glm::normalize(glm::cross(up, L)); // 右方向 因为使用左手系 所以从up叉乘l
	//Vector3f U = glm::cross(L, R); // 相机up方向 |L|=1 |R|=1 U=LxR |LxR|=|L||R|sinθ L和R垂直θ=90°模长为1，所以U本身一定是单位向量 不用归一化
	//// O: 世界坐标系原点
	//// A: 相机坐标系原点
	//// Matrix(0->A) = Rotation(O->A)·Translation(O->A) 
	//// 
	//// 其中 Rotation(A->O) 就是 [L,R,U]  Rotation(O->A)就是Rotation(A->O)的逆矩阵
	//// 又因为[L,R,U]是单位正交矩阵 所以其逆矩阵就是其转置矩阵
	//// 
	//// 其中 Translation(O->A) 是 世界坐标系 到 相机坐标系 
	//// 需要的是 世界坐标系位置 减去 世界原点到相机原点的距离
	//Matrix4x4 viewMatrix = glm::transpose(Matrix4x4(
	//	R.x, R.y, R.z, 0.0,
	//	U.x, U.y, U.z, 0.0,
	//	L.x, L.y, L.z, 0.0,
	//	0.0, 0.0, 0.0, 1.0
	//)) * Matrix4x4(
	//	1.0, 0.0, 0.0, 0.0,
	//	0.0, 1.0, 0.0, 0.0,
	//	0.0, 0.0, 1.0, 0.0,
	//	-p.x, -p.y, -p.z, 1.0
	//);


	Matrix4x4 viewMatrix = glm::lookAtLH(p, target, up);

	Matrix4x4 projMatrix = glm::perspectiveFovLH_ZO(fov, static_cast<float>(W), static_cast<float>(H), near, far);

	// 求视口变换矩阵
	// [-1,1] 缩放到 [0,W] 所以有 x_viewport = (x_ndc + 1) / 2 * W = W/2*x_ndc + W/2  
	// [-1,1] 缩放到 [0,H] 且y轴颠倒 所以有 y_viewport = ( -y_ndc + 1) /2 * H= -W/2*x_ndc + H/2
	// 4x4 * 4*1 = 4*1 所以矩阵为 [[W/2, 0, 0, W/2],[0, -H/2, 0, H/2][0, 0, 1, 0][0, 0, 0, 1]]
	Matrix4x4 viewportMatrix = Matrix4x4(
		W / 2.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -H / 2.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		W / 2.0f, H / 2.0f, 0.0f, 1.0f
	);

	mCombinedMatrix = viewportMatrix * projMatrix * viewMatrix;
	mInverseCombinedMatrix = glm::inverse(mCombinedMatrix);
}

Ray Camera::GetRay(float x, float y) const
{
	Ray ray;
	ray.o = mPosition;

	Vector4f p(x, y, 0.0f, 1.0f);
	Vector4f worldPos = mInverseCombinedMatrix * p;
	worldPos /= worldPos.w;

	ray.d = glm::normalize(Vector3f(worldPos) - mPosition);

	return ray;
}
