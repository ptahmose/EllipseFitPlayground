// EllipseUtils.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ellipseParameters.h"
#include "ellipseUtils.h"
#include "testcases.h"

using namespace EllipseUtils;

static bool IsResultOk(const EllipseFrom5PointsTestCases::TestParameters* ptrTestData, EllipseParameters<double>& ellParams)
{
	const double MaxError = 1e-5;
	if (relativeDifference(ptrTestData->result_x0, ellParams.x0) < MaxError && relativeDifference(ptrTestData->result_y0, ellParams.y0) < MaxError &&
		relativeDifference(ptrTestData->result_a, ellParams.a) < MaxError && relativeDifference(ptrTestData->result_b, ellParams.b) < MaxError)
	{
		if (relativeDifference(ptrTestData->result_theta, ellParams.theta) < MaxError ||
			relativeDifference(ptrTestData->result_theta +  M_PI, ellParams.theta) < MaxError ||
			relativeDifference(ptrTestData->result_theta -  M_PI, ellParams.theta) < MaxError)
		{
			return true;
		}
	}

	return false;
}

static void TestEllipseFrom5Points()
{
	for (int i = 0; ; ++i)
	{
		const EllipseFrom5PointsTestCases::TestParameters* pTestData = EllipseFrom5PointsTestCases::GetTestCase(i);
		if (pTestData == nullptr)
		{
			break;
		}

		EllipseAlgebraicParameters<double> ellAlg = EllipseAlgebraicParameters<double>::CreateFrom5Points(pTestData->points);
		EllipseParameters<double> ellParams = EllipseParameters<double>::FromAlgebraicParameters(ellAlg);

		bool isOk = IsResultOk(pTestData, ellParams);

		printf("x0=%lf y0=%lf a=%lf b=%lf angle=%lf <- %s\n", ellParams.x0, ellParams.y0, ellParams.a, ellParams.b, radToDegree(ellParams.theta), isOk ? "OK" : "FAIL");
	}
}

int main()
{
	TestEllipseFrom5Points();
	return 0;
}

