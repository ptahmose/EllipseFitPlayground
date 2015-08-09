#pragma once

class EllipseFrom5PointsTestCases
{
public:
	struct TestParameters
	{
		double points[10];
		double result_x0, result_y0, result_a, result_b, result_theta;
	};

	static const TestParameters*	GetTestCase(int i);

private:
	static TestParameters tests[];

};

class EllipseLeastSquareFitTestCases
{
public:
	struct TestCase
	{
		const double* pX;
		const double* pY;
		int		      count;

		double result_x0, result_y0, result_a, result_b, result_theta;
	};

	static const TestCase*	GetTestCase(int i);
private:
	static TestCase tests[];
};
