// EllipseUtils.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ellipseParameters.h"
#include "ellipseUtils.h"

using namespace EllipseUtils;

void TestEllipseFrom5Points()
{
	static const struct TestParameters
	{
		double points[10];

	} TestCases[] =
	{
		{
			{
			1408.2722735486361, 490.64057378265801, 1251.1773322832571, 97.576251945392670,	1135.6035395146628,
			921.82905982385944, 1399.5187486964699, 559.41979612991645, 1373.2789494361668, 266.81596353596143
			}
		}
	};

	for (int i = 0; i < sizeof(TestCases) / sizeof(TestCases[0]); ++i)
	{
		const TestParameters* pTestData = TestCases + i;
		EllipseAlgebraicParameters<double> ellAlg = EllipseAlgebraicParameters<double>::CreateFrom5Points(pTestData->points);
		EllipseParameters<double> ellParams = EllipseParameters<double>::FromAlgebraicParameters(ellAlg);

		printf("x0=%lf y0=%lf a=%lf b=%lf angle=%lf", ellParams.x0, ellParams.y0, ellParams.a, ellParams.b, radToDegree(ellParams.theta));
	}
}

int main()
{
	TestEllipseFrom5Points();
	return 0;
}

