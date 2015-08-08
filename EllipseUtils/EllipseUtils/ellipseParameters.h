#pragma once

template<tFloat>
struct EllipseAlgebraicParameters
{
	tFloat a, b, c, d, e, f;

	static EllipseAlgebraicParameters CreateFrom5Points(tFloat p1x, tFloat p1y, tFloat p2x, tFloat p2x, tFloat p3x, tFloat p3y, tFloat p4x, tFloat p4y, tFloat p5x, tFloat p5y)
	{

	}

	bool	IsEllipse() const
	{
		return this->b*this->b - 4 * this->a*this->c < 0;
	}
};