// EllipseUtils.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "optionparser.h"
#include <iostream>
#include "ellipseParameters.h"
#include "ellipseUtils.h"
#include "testcases.h"
#include "writeSVG.h"

using namespace EllipseUtils;

static bool IsResultOk(const EllipseFrom5PointsTestCases::TestParameters* ptrTestData, EllipseParameters<double>& ellParams)
{
	const double MaxError = 1e-5;
	if (relativeDifference(ptrTestData->result_x0, ellParams.x0) < MaxError && relativeDifference(ptrTestData->result_y0, ellParams.y0) < MaxError &&
		relativeDifference(ptrTestData->result_a, ellParams.a) < MaxError && relativeDifference(ptrTestData->result_b, ellParams.b) < MaxError)
	{
		if (relativeDifference(ptrTestData->result_theta, ellParams.theta) < MaxError ||
			relativeDifference(ptrTestData->result_theta + M_PI, ellParams.theta) < MaxError ||
			relativeDifference(ptrTestData->result_theta - M_PI, ellParams.theta) < MaxError)
		{
			return true;
		}
	}

	return false;
}

static std::string GenerateFilenameForSvg(const char* szFilename, int i)
{
	// find the last dot
	const char* lastdot = strrchr(szFilename, '.');
	std::string str;
	str.append(szFilename, lastdot - szFilename);
	char sz[20];
	_itoa_s(i, sz, 10);
	str.append(sz);
	str.append(lastdot);
	return str;
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

static const char* _5POINTTESTOPTION = "5pointtest";

static option::ArgStatus CommandArgRequired(const option::Option& option, bool msg)
{
	if (option.arg != 0)
	{
		if (strcmp(option.arg, _5POINTTESTOPTION) == 0)
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


	return 0;
}

