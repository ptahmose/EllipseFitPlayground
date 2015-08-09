#pragma once

void write_svg_five_points_and_ellipse(const char* szFilename, int xsize, int ysize, std::function<bool(double&, double&, bool&)> getPoints, double x0, double y0, double a, double b, double theta);
void write_svg_five_points_and_ellipse(const char* szFilename, std::function<bool(double&, double&, bool&)> getPoints, double x0, double y0, double a, double b, double theta);