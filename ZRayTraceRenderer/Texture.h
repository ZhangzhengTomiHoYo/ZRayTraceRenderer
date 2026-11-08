#pragma once
#include <vector>
#include <string>
#include "Common.h"

class Texture
{
public:
	Texture(int width, int height, int channel, uint8_t* data) : mWidth(width), mHeight(height), mChannel(channel)
	{
		mData.resize(width * height * channel);
		memcpy(mData.data(), data, width * height * channel);
	}

	// 工厂方法 因为如果在构造函数做 一定会创建出texture 但是错误处理该如何做 如果读取数据失败 本不应该创出texture
	static Texture* CreateFromFile(const std::string& filePath);
	
	template<typename T>
	T GetPixel(int x, int y)
	{
		// 坐标合法
		// 寻址模式：钳位模式
		//x = glm::clamp(x, 0, mWidth - 1);
		//y = glm::clamp(y, 0, mHeight - 1);

		// 寻址模式：重复模式
		x = x % mWidth;
		if (x < 0)
		{
			x += mWidth;
		}
		y = y % mHeight;
		if (y < 0)
		{
			y += mHeight;
		}

		// RGB RGB ... RGB 像素这样存储
		// xy 转 vector 上的index
		int index = (y * mWidth + x) * mChannel;

		// 0-255 转 0-1
		if constexpr (std::is_same_v<T, float>)  // constexpr 静态分支 编译器就确定 模板的规则 C++17新加的语法 if constexpr语句
		{
			return mData[index] / 255.0f;
		}
		else
		{	
			// 少了 补0
			// 多了 截断

			// 先填充0 T有可能是 vec2 vec3 vec4
			T result(0);
			int count = std::min(T::length(), mChannel);
			for (int i = 0; i < count; i++)
			{
				result[i] = mData[index + i] / 255.0f;
			}
			return result;
		}
	}

	template<typename T>
	T Sample(Vector2f uv)
	{
		// 0~1 uv坐标 转 xy 像素坐标
		// 四舍五入
		// 临近点采样
		//int x = std::round(uv.x * mWidth);
		//int y = std::round(uv.y * mHeight);

		// 线性插值采样
		// 是算到像素中心的距离 而不是像素边的距离 所以减0.5
		float fx = uv.x * mWidth - 0.5f;  
		float fy = uv.y * mHeight - 0.5f;

		int x = (int)std::floor(fx);
		int y = (int)std::floor(fy);

		float k1 = fx - x;
		float k2 = fy - y;

		T c1 = GetPixel<T>(x, y);
		T c2 = GetPixel<T>(x+1, y);
		T c3 = GetPixel<T>(x, y+1);
		T c4 = GetPixel<T>(x+1, y+1);

		T c5 = glm::mix(c1, c2, k1);
		T c6 = glm::mix(c3, c4, k1);

		return glm::mix(c5, c6, k2);
	}

private:
	int mWidth;
	int mHeight;
	int mChannel;
	std::vector<uint8_t> mData;
};

