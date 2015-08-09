#include "stdafx.h"
#include "writeSVG.h"

void write_svg_five_points_and_ellipse(const char* szFilename, std::function<bool(double&, double&, bool&)> getPoints, double x0, double y0, double a, double b, double theta)
{
	int xsize = (int)(abs(x0) + (std::max)(abs(a), abs(b)) + 0.5);
	int ysize = (int)(abs(y0) + (std::max)(abs(a), abs(b)) + 0.5);
	write_svg_five_points_and_ellipse(szFilename, xsize, ysize, getPoints, x0, y0, a, b, theta);
}

void write_svg_five_points_and_ellipse(const char* szFilename, int xsize, int ysize, std::function<bool(double&, double&, bool&)> getPoints, double x0, double y0, double a, double b, double theta)
{
	FILE * svg;

	if (xsize <= 0 || ysize <= 0)
		throw std::logic_error("Error: invalid image size in write_svg.");

	/* open file */
	/*if (wcscmp(szwFilename, L"-") == 0) svg = stdout;
	else svg = _wfopen(szwFilename, L"w");*/
	fopen_s(&svg, szFilename, "w");
	if (svg == NULL) throw std::logic_error("Error: unable to open SVG output file.");

	/* write SVG header */
	fprintf(svg, "<?xml version=\"1.0\" standalone=\"no\"?>\n");
	fprintf(svg, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n");
	fprintf(svg, " \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
	fprintf(svg, "<svg width=\"%dpx\" height=\"%dpx\" ", xsize, ysize);
	fprintf(svg, "version=\"1.1\"\n xmlns=\"http://www.w3.org/2000/svg\" ");
	fprintf(svg, "xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n");

	/* write line segments */
	//for (i = 0; i<n; i++)
	const double radiusPoint = 2;
	for (;;)
	{
		double x, y; bool isSpecial;
		bool b = getPoints(x, y, isSpecial);
		if (b == false)
		{
			break;
		}

		if (isSpecial == false)
		{
			fprintf(svg, "<circle cx=\"%f\" cy=\"%f\" r=\"%f\" fill=\"black\" />\n", x, y, radiusPoint);
		}
		else
		{
			fprintf(svg, "<circle cx=\"%f\" cy=\"%f\" r=\"%f\" fill=\"red\" />\n", x, y, radiusPoint * 2);
		}
	}

	fprintf(svg,
		"<g transform=\"translate(%lf %lf)  rotate(%lf)\">\n"
		"<ellipse cx=\"%lf\" cy= \"%lf\" rx=\"%lf\" ry=\"%lf\" fill=\"none\" stroke=\"purple\" stroke-width=\"3\" />\n"
		"</g>\n",
		x0, y0,
		180 * theta / M_PI,
		0.0, 0.0, a, b);


	/* close SVG file */
	fprintf(svg, "</svg>\n");
	if (svg != stdout && fclose(svg) == EOF)
		throw std::logic_error("Error: unable to close file while writing SVG file.");
}