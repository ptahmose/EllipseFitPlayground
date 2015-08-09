// EllipseUtils.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "optionparser.h"
#include <iostream>
#include "ellipseParameters.h"
#include "ellipseUtils.h"
#include "testcases.h"
#include "leastSquareEllipseFit.h"
#include "writeSVG.h"

using namespace EllipseUtils;

static bool IsResultOk(double result_x0, double result_y0, double result_a, double result_b, double result_theta, EllipseParameters<double>& ellParams,double maxError)
{
	if (relativeDifference(result_x0, ellParams.x0) < maxError && relativeDifference(result_y0, ellParams.y0) < maxError &&
		relativeDifference(result_a, ellParams.a) < maxError && relativeDifference(result_b, ellParams.b) < maxError)
	{
		if (relativeDifference(result_theta, ellParams.theta) < maxError ||
			relativeDifference(result_theta + M_PI, ellParams.theta) < maxError ||
			relativeDifference(result_theta - M_PI, ellParams.theta) < maxError)
		{
			return true;
		}
	}

	return false;
}

static bool IsResultOk(const EllipseFrom5PointsTestCases::TestParameters* ptrTestData, EllipseParameters<double>& ellParams)
{
	const double MaxError = 1e-5;
	return IsResultOk(ptrTestData->result_x0, ptrTestData->result_y0, ptrTestData->result_a, ptrTestData->result_b, ptrTestData->result_theta, ellParams, MaxError);
}

static bool IsResultOk(const EllipseLeastSquareFitTestCases::TestCase* ptrTestCaseData, EllipseParameters<double>& ellParams)
{
	const double MaxError = 1e-5;
	return IsResultOk(ptrTestCaseData->result_x0, ptrTestCaseData->result_y0, ptrTestCaseData->result_a, ptrTestCaseData->result_b, ptrTestCaseData->result_theta, ellParams, MaxError);
}

static std::string GenerateFilenameForSvg(const char* szFilename, int i)
{
	// find the last dot
	const char* lastdot = strrchr(szFilename, '.');
	std::string name;
	if (lastdot != nullptr)
	{
		if (_stricmp(lastdot + 1, "svg") == 0)
		{
			name.append(szFilename, lastdot - szFilename);
		}
		else
		{
			name.append(szFilename);
		}
	}
	else
	{
		name.append(szFilename);
	}

	std::ostringstream oss;
	oss << name << i << ".svg";
	return oss.str();
}

static bool TestEllipseFrom5Points(const char* szFilename)
{
	bool allOk = true;
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

		if (!isOk)
		{
			allOk = false;
		}

		if (szFilename != nullptr)
		{
			auto filename = GenerateFilenameForSvg(szFilename, i);
			int counter = 0;
			write_svg_five_points_and_ellipse(
				filename.c_str(),
				[&](double& x, double& y, bool& isSpecial)->bool
			{
				if (counter < 5)
				{
					x = pTestData->points[counter * 2];
					y = pTestData->points[counter * 2 + 1];
					isSpecial = true;
					++counter;
					return true;
				}

				return false;
			},
				ellParams.x0, ellParams.y0, ellParams.a, ellParams.b, ellParams.theta);
		}
	}

	return allOk;
}


static bool TestLeastSquareFit()
{
	//std::vector<double> posX{ 1191.890202, 1202.992439, 1251.177332, 1290.494167, 1316.046031, 1330.852813, 1352.380307, 1373.278949, 1365.819934, 1390.200345, 1396.257962, 1405.251032, 1408.272274, 1399.518749, 1375.738192, 1354.890642, 1344.181910, 1329.239047, 1291.598685, 1272.002922, 1236.409267, 1193.330425, 1135.603540, 1124.718146, 1058.339563, 1014.500301, 989.403974, 891.826663, 848.007368 };
	//std::vector<double> posY{ 56.850882, 63.318848, 97.576252, 138.239738, 169.077405, 194.235056, 230.361399, 266.815964, 315.499247, 325.761950, 335.514097, 398.074500, 490.640574, 559.419796, 633.170436, 687.140494, 709.254771, 730.149767, 783.886493, 811.988798, 850.083598, 883.423782, 921.829060, 930.230122, 953.948848, 962.635142, 965.784117, 971.813619, 963.767163 };
	bool allOk = true;
	for (int i = 0;; ++i)
	{
		auto testParams = EllipseLeastSquareFitTestCases::GetTestCase(i);
		if (testParams == nullptr)
		{
			break;
		}

		LeastSquareEllipseFitter<double>::PointAccessorFromTwoArrays accessor(testParams->pX, testParams->pY, testParams->count);
		auto result = LeastSquareEllipseFitter<double>::Fit(accessor);
		EllipseParameters<double> ellParams = EllipseParameters<double>::FromAlgebraicParameters(result);

		bool isOk = IsResultOk(testParams, ellParams);
		printf("x0=%lf y0=%lf a=%lf b=%lf angle=%lf <- %s\n", ellParams.x0, ellParams.y0, ellParams.a, ellParams.b, radToDegree(ellParams.theta), isOk ? "OK" : "FAIL");

		if (!isOk)
		{
			allOk = false;
		}
	}

	return allOk;
}


static const char* _5POINTTESTOPTION = "5pointtest";
static const char* LEASTSQUAREELLIPSETESTOPTION = "leastsquarefittest";

static option::ArgStatus CommandArgRequired(const option::Option& option, bool msg)
{
	if (option.arg != 0)
	{
		if (strcmp(option.arg, _5POINTTESTOPTION) == 0 ||
			strcmp(option.arg, LEASTSQUAREELLIPSETESTOPTION) == 0)
		{
			return option::ARG_OK;
		}
	}

	return option::ARG_ILLEGAL;
}

static option::ArgStatus FilenameArgRequired(const option::Option& option, bool msg)
{
	if (option.arg != 0)
	{
		return option::ARG_OK;
	}

	return option::ARG_ILLEGAL;
}

enum  optionIndex { UNKNOWN, HELP, COMMAND, SVGOUTPUT };
const option::Descriptor usage[] =
{
	{ UNKNOWN, 0,"" , ""    ,option::Arg::None, "USAGE: example [options]\n\n"
	"Options:" },
	{ HELP,    0,"" , "help",option::Arg::None, "  --help  \tPrint usage and exit." },
	{ COMMAND,    0,"c", "command",CommandArgRequired, "  --command, -c  \tspecifies command." },
	{ SVGOUTPUT,  0,"s" ,  "svg"   ,FilenameArgRequired, "  --svg, -s  \tspecifies filename for SVG-output." },
	{ 0,0,0,0,0,0 }
};

int main(int argc, char* argv[])
{
	argc -= (argc > 0); argv += (argc > 0); // skip program name argv[0] if present
	option::Stats  stats(usage, argc, argv);
	auto options = std::vector<option::Option>(stats.options_max);
	options.resize(stats.options_max);
	auto  buffer = std::vector<option::Option>(stats.buffer_max);
	buffer.resize(stats.buffer_max);
	option::Parser parse(usage, argc, argv, options.data(), buffer.data());

	if (!options[COMMAND])
	{
		return EXIT_FAILURE;
	}

	const char* command = options[COMMAND].arg;
	if (strcmp(command, _5POINTTESTOPTION) == 0)
	{
		const char* filename = nullptr;
		if (options[SVGOUTPUT])
		{
			filename = options[SVGOUTPUT].arg;
		}

		TestEllipseFrom5Points(filename);
	}
	else if (strcmp(command, LEASTSQUAREELLIPSETESTOPTION) == 0)
	{
		TestLeastSquareFit();
	}


	return 0;
}

