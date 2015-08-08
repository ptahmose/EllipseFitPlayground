#pragma once

class EllipseFrom5PointsTestCases
{
public:
	struct TestParameters
	{
		double points[10];
		double result_x0, result_y0, result_a, result_b, result_theta;
	};

	static TestParameters*	GetTestCase(int i);

private:
	static TestParameters tests[];

};
