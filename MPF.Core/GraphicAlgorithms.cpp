#include "stdafx.h"
#include "../include/visual/GraphicAlgorithms.h"
#include <emmintrin.h>
#include "../include/visual/Line.h"

using namespace MPF;
using namespace MPF::Visual;

#pragma warning(disable : 4996)  // this function or variable may be unsafe... yeah, right

#define _INLINE inline

namespace RTTL {
	// It is an empty implementation for stand-alone app. 
	class AtomicCounter  {
	private:
		int m_counter;

	public:
		AtomicCounter() {
			m_counter = -1;
		}

		_INLINE int inc() {
			return ++m_counter;
		}
	};
};

#include "MLAA.hxx"

int pixelx, pixely, pixelmode = 0;

void GraphicAlgorithms::MLAA(color_t* data, uint32_t width, uint32_t height)
{
	RTTL::AtomicCounter job;
	::MLAA((unsigned int*)data, NULL, width, height, job);
}

inline double round(double x)
{
	return std::floor(x + 0.5);
}

inline double fpart(double x)
{
	return x - std::floor(x);
}

inline double rfpart(double x)
{
	return 1.0 - fpart(x);
}

inline void MixColor(argb_color& col1, argb_color col2)
{
	auto p = col2.alpha / 255.f;

	col1.alpha = col2.alpha * p + col1.alpha * (1 - p);
	col1.red = col2.red * p + col1.red * (1 - p);
	col1.green = col2.green * p + col1.green * (1 - p);
	col1.blue = col2.blue * p + col1.blue * (1 - p);
}

inline void MixColor(color_t& col1, argb_color col2)
{
	argb_color col = col1;
	MixColor(col, col2);
	col1 = col;
}

void GraphicAlgorithms::DrawLine_WuXiaolin(color_t* data, uint32_t stride,
	uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, color_t color)
{
	if (DrawSpecialLine(data, stride, x1, y1, x2, y2, color)) return;

	double dx = (double)x2 - (double)x1;
	double dy = (double)y2 - (double)y1;
	const auto ax = std::fabs(dx);
	const auto ay = std::fabs(dy);
	std::function<void(uint32_t, uint32_t, double)> plot;
	argb_color col;
	col.col = color;

	if (ax < ay)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
		std::swap(dx, dy);

		plot = [=](uint32_t y, uint32_t x, double p)
		{
			argb_color col1;
			col1.col = data[y * stride + x];

			data[y * stride + x] = MixColor(col, col1, p).col;
		};
	}
	else
	{
		plot = [=](uint32_t x, uint32_t y, double p)
		{
			argb_color col1;
			col1.col = data[y * stride + x];

			data[y * stride + x] = MixColor(col, col1, p).col;
		};
	}

	if (x2 < x1)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}
	const auto gradient = dy / dx;
	auto xEnd = std::round(x1);
	auto yEnd = y1 + gradient * (xEnd - x1);
	auto xGap = rfpart(x1 + 0.5);

	const auto xpxl1 = (uint32_t)xEnd;
	const auto ypxl1 = (uint32_t)std::floor(yEnd);
	plot(xpxl1, ypxl1, rfpart(yEnd) * xGap);
	plot(xpxl1, ypxl1 + 1, fpart(yEnd) * xGap);

	auto yInter = yEnd + gradient;

	xEnd = std::round(x2);
	yEnd = y2 + gradient * (xEnd - x2);
	xGap = fpart(x2 + 0.5);
	const auto xpxl2 = (uint32_t)xEnd;
	const auto ypxl2 = (uint32_t)std::floor(yEnd);
	plot(xpxl2, ypxl2, rfpart(yEnd) * xGap);
	plot(xpxl2, ypxl2 + 1, fpart(yEnd) * xGap);

	for (uint32_t x = xpxl1 + 1; x < xpxl2; x++)
	{
		plot(x, std::floor(yInter), rfpart(yInter));
		plot(x, std::floor(yInter) + 1, fpart(yInter));

		yInter += gradient;
	}
}

void GraphicAlgorithms::DrawLine_WuXiaolin(color_t* data, uint32_t stride, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2,
	float u1, float v1, float u2, float v2, const Brush& brush)
{
	if (DrawSpecialLine(data, stride, x1, y1, x2, y2, u1, v1, u2, v2, brush)) return;

	double dx = (double)x2 - (double)x1;
	double dy = (double)y2 - (double)y1;
	const auto ax = std::fabs(dx);
	const auto ay = std::fabs(dy);
	std::function<void(uint32_t, uint32_t, float, float, double)> plot;

	if (ax < ay)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
		std::swap(dx, dy);
		std::swap(u1, v1);
		std::swap(u2, v2);

		plot = [=, &brush](uint32_t y, uint32_t x, float v, float u, double p)
		{
			argb_color col1, col;
			col1.col = data[y * stride + x];
			col.col = brush.TakeSample(u, v);

			data[y * stride + x] = MixColor(col, col1, p).col;
		};
	}
	else
	{
		plot = [=, &brush](uint32_t x, uint32_t y, float u, float v, double p)
		{
			argb_color col1, col;
			col1.col = data[y * stride + x];
			col.col = brush.TakeSample(u, v);

			data[y * stride + x] = MixColor(col, col1, p).col;
		};
	}

	if (x2 < x1)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
		std::swap(u1, u2);
		std::swap(v1, v2);
	}
	const auto gradient = dy / dx;
	const auto gradientU = (u2 - u1) / dx;
	const auto gradientV = (v2 - v1) / dx;
	auto xEnd = std::round(x1);
	auto yEnd = y1 + gradient * (xEnd - x1);
	auto xGap = rfpart(x1 + 0.5);

	const auto xpxl1 = (uint32_t)xEnd;
	const auto ypxl1 = (uint32_t)std::floor(yEnd);
	plot(xpxl1, ypxl1, u1, v1, rfpart(yEnd) * xGap);
	plot(xpxl1, ypxl1 + 1, u1, v1, fpart(yEnd) * xGap);

	auto yInter = yEnd + gradient;
	auto vInter = v1 + gradientV * (xEnd - x1) + gradientV;

	xEnd = std::round(x2);
	yEnd = y2 + gradient * (xEnd - x2);
	xGap = fpart(x2 + 0.5);
	const auto xpxl2 = (uint32_t)xEnd;
	const auto ypxl2 = (uint32_t)std::floor(yEnd);
	plot(xpxl2, ypxl2, u2, v2, rfpart(yEnd) * xGap);
	plot(xpxl2, ypxl2 + 1, u2, v2, fpart(yEnd) * xGap);

	auto upxl1 = u1;
	for (uint32_t x = xpxl1 + 1; x < xpxl2; x++)
	{
		plot(x, std::floor(yInter), upxl1, vInter, rfpart(yInter));
		plot(x, std::floor(yInter) + 1, upxl1, vInter, fpart(yInter));

		yInter += gradient;
		upxl1 += gradientU;
		vInter += gradientV;
	}
}

void GraphicAlgorithms::DrawLine_Bresenham(color_t* data, uint32_t stride,
	int x1, int y1, int x2, int y2, color_t color)
{
	int dx, dy, p, const1, const2, x, y, inc;

	if (DrawSpecialLine(data, stride, x1, y1, x2, y2, color)) return;

	int steep = (abs(y2 - y1) > abs(x2 - x1)) ? 1 : 0;
	if (steep == 1)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}
	if (x1 > x2)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}
	dx = abs(x2 - x1);
	dy = abs(y2 - y1);
	p = 2 * dy - dx;
	const1 = 2 * dy;
	const2 = 2 * (dy - dx);
	x = x1;
	y = y1;

	inc = (y1 < y2) ? 1 : -1;
	while (x <= x2)
	{
		if (steep == 1)
			data[x * stride + y] = color;
		else
			data[y * stride + x] = color;
		x++;
		if (p < 0)
			p += const1;
		else
		{
			p += const2;
			y += inc;
		}
	}
}

void GraphicAlgorithms::FillTriangle_Solid(color_t* data, uint32_t stride, uint32_t x1, uint32_t y1,
	uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, color_t color)
{
	//保证 y1 是最小的
	if (y1 > y2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}
	if (y1 > y3)
	{
		std::swap(x1, x3);
		std::swap(y1, y3);
	}
	//保证 y3 是最大的
	if (y3 < y2)
	{
		std::swap(x2, x3);
		std::swap(y2, y3);
	}
	//y1 ~ y2, y1 ~ y3, y2 ~ y3;
	Line lines[] =
	{
		Line(Point(x1, y1), Point(x2, y2)),
		Line(Point(x1, y1), Point(x3, y3)),
		Line(Point(x2, y2), Point(x3, y3))
	};
	auto start = y1 * stride;
	for (uint32_t y = y1; y < y2; y++)
	{
		uint32_t minX = lines[0].GetX(y);
		uint32_t maxX = lines[1].GetX(y);
		//
		if (minX > maxX)
		{
			std::swap(minX, maxX);
		}
		for (uint32_t x = minX; x < maxX; x++)
		{
			data[start + x] = color;
		}
		start += stride;
	}

	for (uint32_t y = y2; y < y3; y++)
	{
		uint32_t minX = lines[2].GetX(y);
		uint32_t maxX = lines[1].GetX(y);
		//
		if (minX > maxX)
		{
			std::swap(minX, maxX);
		}
		for (uint32_t x = minX; x < maxX; x++)
		{
			data[start + x] = color;
		}
		start += stride;
	}
}

void GraphicAlgorithms::FillTriangle(color_t* data, uint32_t stride, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2,
	uint32_t x3, uint32_t y3, float u1, float v1, float u2, float v2, float u3, float v3, const Brush& brush)
{
	//保证 y1 是最小的
	if (y1 > y2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
		std::swap(u1, u2);
		std::swap(v1, v2);
	}
	if (y1 > y3)
	{
		std::swap(x1, x3);
		std::swap(y1, y3);
		std::swap(u1, u3);
		std::swap(v1, v3);
	}
	//保证 y3 是最大的
	if (y3 < y2)
	{
		std::swap(x2, x3);
		std::swap(y2, y3);
		std::swap(u2, u3);
		std::swap(v2, v3);
	}
	//y1 ~ y2, y1 ~ y3, y2 ~ y3;
	Line lines[] =
	{
		Line(Point(x1, y1, u1, v1), Point(x2, y2, u2, v2)),
		Line(Point(x1, y1, u1, v1), Point(x3, y3, u3, v3)),
		Line(Point(x2, y2, u2, v2), Point(x3, y3, u3, v3))
	};
	auto start = y1 * stride;
	for (uint32_t y = y1; y < y2; y++)
	{
		auto minX = lines[0].GetX(y);
		auto maxX = lines[1].GetX(y);
		auto minUV = lines[0].GetUV(y);
		auto maxUV = lines[1].GetUV(y);
		//
		if (minX > maxX)
		{
			std::swap(minX, maxX);
			std::swap(minUV, maxUV);
		}
		auto iminX = std::roundf(minX);
		auto imaxX = std::roundf(maxX);
		const auto gradU = (maxX - minX) ? (maxUV.first - minUV.first) / (maxX - minX) : 0.f;
		const auto gradV = (maxX - minX) ? (maxUV.second - minUV.second) / (maxX - minX) : 0.f;
		for (uint32_t x = iminX, i = 0; x < imaxX; x++, i++)
		{
			auto u = gradU * i + minUV.first;
			auto v = gradV * i + minUV.second;
			::MixColor(data[start + x], brush.TakeSample(u, v));
		}
		start += stride;
	}

	for (uint32_t y = y2; y < y3; y++)
	{
		auto minX = lines[2].GetX(y);
		auto maxX = lines[1].GetX(y);
		auto minUV = lines[2].GetUV(y);
		auto maxUV = lines[1].GetUV(y);
		//
		if (minX > maxX)
		{
			std::swap(minX, maxX);
			std::swap(minUV, maxUV);
		}
		auto iminX = std::roundf(minX);
		auto imaxX = std::roundf(maxX);
		const auto gradU = (maxX - minX) ? (maxUV.first - minUV.first) / (maxX - minX) : 0.f;
		const auto gradV = (maxX - minX) ? (maxUV.second - minUV.second) / (maxX - minX) : 0.f;
		for (uint32_t x = iminX, i = 0; x < imaxX; x++, i++)
		{
			auto u = gradU * i + minUV.first;
			auto v = gradV * i + minUV.second;
			::MixColor(data[start + x], brush.TakeSample(u, v));
		}
		start += stride;
	}
}

bool GraphicAlgorithms::DrawSpecialLine(color_t* data, uint32_t stride, int x1, int y1, int x2, int y2, color_t color)
{
	//纵向
	if (x1 == x2)
	{
		//保证 y1 < y2
		if (y1 > y2)
		{
			std::swap(y1, y2);
		}
		auto curPixel = y1 * stride + x1;
		for (uint32_t y = y1; y <= y2; y++)
		{
			data[curPixel] = color;
			curPixel += stride;
		}
	}
	//横向
	else if (y1 == y2)
	{
		//保证 x1 < x2
		if (x1 > x2)
		{
			std::swap(x1, x2);
		}
		const auto start = y1 * stride;
		for (uint32_t x = x1; x <= x2; x++)
		{
			data[start + x] = color;
		}
	}
	//45度
	else if (x1 - x2 == y1 - y2)
	{
		//保证 x1 < x2（y1 < y2）
		if (x1 > x2)
		{
			std::swap(x1, x2);
			std::swap(y1, y2);
		}
		auto curPixel = y1 * stride + x1;
		for (uint32_t y = y1; y <= y2; y++)
		{
			data[curPixel] = color;
			curPixel += stride + 1;
		}
	}
	//135度
	else if (x1 - x2 == y2 - y1)
	{
		//保证 y1 < y2（x1 > x2）
		if (y1 > y2)
		{
			std::swap(x1, x2);
			std::swap(y1, y2);
		}
		auto curPixel = y1 * stride + x1;
		for (uint32_t y = y1; y <= y2; y++)
		{
			data[curPixel] = color;
			curPixel += stride - 1;
		}
	}
	//其他情况
	else
	{
		return false;
	}
	return true;
}

bool GraphicAlgorithms::DrawSpecialLine(color_t* data, uint32_t stride, int x1, int y1,
	int x2, int y2, float u1, float v1, float u2, float v2, const Brush& brush)
{
	//纵向
	if (x1 == x2)
	{
		//保证 y1 < y2
		if (y1 > y2)
		{
			std::swap(y1, y2);
			std::swap(u1, u2);
			std::swap(v1, v2);
		}
		auto curPixel = y1 * stride + x1;
		auto const gradient = (y2 - y1) ? (v2 - v1) / (y2 - y1) : 0.f;
		for (uint32_t y = y1; y <= y2; y++)
		{
			data[curPixel] = brush.TakeSample(u1, v1);
			curPixel += stride;
			v1 += gradient;
		}
	}
	//横向
	else if (y1 == y2)
	{
		//保证 x1 < x2
		if (x1 > x2)
		{
			std::swap(x1, x2);
			std::swap(u1, u2);
			std::swap(v1, v2);
		}
		const auto start = y1 * stride;
		auto const gradient = (u2 - u1) / (x2 - x1);
		for (uint32_t x = x1; x <= x2; x++)
		{
			data[start + x] = brush.TakeSample(u1, v1);
			u1 += gradient;
		}
	}
	//45度
	else if (x1 - x2 == y1 - y2)
	{
		//保证 x1 < x2（y1 < y2）
		if (x1 > x2)
		{
			std::swap(x1, x2);
			std::swap(y1, y2);
			std::swap(u1, u2);
			std::swap(v1, v2);
		}
		auto curPixel = y1 * stride + x1;
		auto const gradientX = (u2 - u1) / (y2 - y1);
		auto const gradientY = (v2 - v1) / (y2 - y1);
		for (uint32_t y = y1; y <= y2; y++)
		{
			data[curPixel] = brush.TakeSample(u1, v1);
			curPixel += stride + 1;
			u1 += gradientX;
			v1 += gradientY;
		}
	}
	//135度
	else if (x1 - x2 == y2 - y1)
	{
		//保证 y1 < y2（x1 > x2）
		if (y1 > y2)
		{
			std::swap(x1, x2);
			std::swap(y1, y2);
			std::swap(u1, u2);
			std::swap(v1, v2);
		}
		auto curPixel = y1 * stride + x1;
		auto const gradientX = (u2 - u1) / (y2 - y1);
		auto const gradientY = (v2 - v1) / (y2 - y1);
		for (uint32_t y = y1; y <= y2; y++)
		{
			data[curPixel] = brush.TakeSample(u1, v1);
			curPixel += stride - 1;
			u1 += gradientX;
			v1 += gradientY;
		}
	}
	//其他情况
	else
	{
		return false;
	}
	return true;
}

void GraphicAlgorithms::FillQuad(color_t* data, uint32_t stride, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2,
	uint32_t x3, uint32_t y3, uint32_t x4, uint32_t y4, float u1, float v1, float u2, float v2,
	float u3, float v3, float u4, float v4, const Brush& brush)
{
	FillTriangle(data, stride, x1, y1, x2, y2, x3, y3, u1, v1, u2, v2, u3, v3, brush);
	FillTriangle(data, stride, x3, y3, x4, y4, x1, y1, u3, v3, u4, v4, u1, v1, brush);
}