#pragma once
#include "Geometry.h"

NS_MPF
NS_VSL

//������
class Triangle : public Geometry
{
public:
	//����һ��������
	MPF_API explicit Triangle(const Point& pointA, const Point& pointB, const Point& pointC);

	//��ȡ��������
	MPF_API virtual std::vector<Point> GetPoints() const;

	//��ȡ����
	DECLARE_GETTYPE(Triangle)
private:
	Point pointA;
	Point pointB;
	Point pointC;

	DECLARE_TYPE(Triangle)
};

NS_ED
NS_ED