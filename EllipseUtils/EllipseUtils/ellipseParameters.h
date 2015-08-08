#pragma once

#include <limits>

namespace EllipseUtils
{
	template<typename tFloat>
	struct EllipseAlgebraicParameters
	{
	public:
		tFloat a, b, c, d, e, f;

		static EllipseAlgebraicParameters CreateFrom5Points(tFloat p1x, tFloat p1y, tFloat p2x, tFloat p2y, tFloat p3x, tFloat p3y, tFloat p4x, tFloat p4y, tFloat p5x, tFloat p5y)
		{
			tFloat pp0[3] = { p1x, p1y, 1 };
			tFloat pp1[3] = { p2x, p2y, 1 };
			tFloat pp2[3] = { p3x, p3y, 1 };
			tFloat pp3[3] = { p4x, p4y, 1 };
			tFloat pp4[3] = { p5x, p5y, 1 };
			return EllipseAlgebraicParameters::FromPoints(pp0, pp1, pp2, pp3, pp4);
		}

		static EllipseAlgebraicParameters CreateFrom5Points(const tFloat* points)
		{
			tFloat pp0[3] = { points[0], points[1], 1 };
			tFloat pp1[3] = { points[2], points[3], 1 };
			tFloat pp2[3] = { points[4], points[5], 1 };
			tFloat pp3[3] = { points[6], points[7], 1 };
			tFloat pp4[3] = { points[8], points[9], 1 };
			return EllipseAlgebraicParameters::FromPoints(pp0, pp1, pp2, pp3, pp4);
		}

		bool	IsEllipse() const
		{
			return this->b*this->b - 4 * this->a*this->c < 0;
		}

	private:
		static EllipseAlgebraicParameters FromPoints(tFloat p0[3], tFloat p1[3], tFloat p2[3], tFloat p3[3], tFloat p4[3])
		{
			tFloat L0[3], L1[3], L2[3], L3[3];
			tFloat A, B, C, Q[3];
			tFloat a1, a2, b1, b2, c1, c2;
			tFloat x0, x4, y0, y4, w0, w4;
			tFloat aa, bb, cc, dd, ee, ff;
			tFloat y4w0, w4y0, w4w0, y4y0, x4w0, w4x0, x4x0, y4x0, x4y0;
			tFloat a1a2, a1b2, a1c2, b1a2, b1b2, b1c2, c1a2, c1b2, c1c2;

			L0[0] = p0[1] * p1[2] - p0[2] * p1[1]; 	L0[1] = p0[2] * p1[0] - p0[0] * p1[2]; L0[2] = p0[0] * p1[1] - p0[1] * p1[0];
			L1[0] = p1[1] * p2[2] - p1[2] * p2[1]; L1[1] = p1[2] * p2[0] - p1[0] * p2[2]; 	L1[2] = p1[0] * p2[1] - p1[1] * p2[0];
			L2[0] = p2[1] * p3[2] - p2[2] * p3[1]; 	L2[1] = p2[2] * p3[0] - p2[0] * p3[2]; 	L2[2] = p2[0] * p3[1] - p2[1] * p3[0];
			L3[0] = p3[1] * p4[2] - p3[2] * p4[1]; 	L3[1] = p3[2] * p4[0] - p3[0] * p4[2]; L3[2] = p3[0] * p4[1] - p3[1] * p4[0];
			Q[0] = L0[1] * L3[2] - L0[2] * L3[1]; Q[1] = L0[2] * L3[0] - L0[0] * L3[2]; Q[2] = L0[0] * L3[1] - L0[1] * L3[0];

			A = Q[0]; B = Q[1]; C = Q[2];
			a1 = L1[0]; b1 = L1[1]; c1 = L1[2];
			a2 = L2[0]; b2 = L2[1]; c2 = L2[2];
			x0 = p0[0]; y0 = p0[1]; w0 = p0[2];
			x4 = p4[0]; y4 = p4[1]; w4 = p4[2];

			y4w0 = y4*w0;
			w4y0 = w4*y0;
			w4w0 = w4*w0;
			y4y0 = y4*y0;
			x4w0 = x4*w0;
			w4x0 = w4*x0;
			x4x0 = x4*x0;
			y4x0 = y4*x0;
			x4y0 = x4*y0;
			a1a2 = a1*a2;
			a1b2 = a1*b2;
			a1c2 = a1*c2;
			b1a2 = b1*a2;
			b1b2 = b1*b2;
			b1c2 = b1*c2;
			c1a2 = c1*a2;
			c1b2 = c1*b2;
			c1c2 = c1*c2;

			aa = -A*a1a2*y4w0 + A*a1a2*w4y0 - B*b1a2*y4w0 - B*c1a2*w4w0 + B*a1b2*w4y0 +
				B*a1c2*w4w0 + C*b1a2*y4y0 + C*c1a2*w4y0 - C*a1b2*y4y0 - C*a1c2*y4w0;

			cc = A*c1b2*w4w0 + A*a1b2*x4w0 - A*b1c2*w4w0 - A*b1a2*w4x0 + B*b1b2*x4w0
				- B*b1b2*w4x0 + C*b1c2*x4w0 + C*b1a2*x4x0 - C*c1b2*w4x0 - C*a1b2*x4x0;

			ff = A*c1a2*y4x0 + A*c1b2*y4y0 - A*a1c2*x4y0 - A*b1c2*y4y0 - B*c1a2*x4x0
				- B*c1b2*x4y0 + B*a1c2*x4x0 + B*b1c2*y4x0 - C*c1c2*x4y0 + C*c1c2*y4x0;

			bb = A*c1a2*w4w0 + A*a1a2*x4w0 - A*a1b2*y4w0 - A*a1c2*w4w0 - A*a1a2*w4x0
				+ A*b1a2*w4y0 + B*b1a2*x4w0 - B*b1b2*y4w0 - B*c1b2*w4w0 - B*a1b2*w4x0
				+ B*b1b2*w4y0 + B*b1c2*w4w0 - C*b1c2*y4w0 - C*b1a2*x4y0 - C*b1a2*y4x0
				- C*c1a2*w4x0 + C*c1b2*w4y0 + C*a1b2*x4y0 + C*a1b2*y4x0 + C*a1c2*x4w0;

			dd = -A*c1a2*y4w0 + A*a1a2*y4x0 + A*a1b2*y4y0 + A*a1c2*w4y0 - A*a1a2*x4y0
				- A*b1a2*y4y0 + B*b1a2*y4x0 + B*c1a2*w4x0 + B*c1a2*x4w0 + B*c1b2*w4y0
				- B*a1b2*x4y0 - B*a1c2*w4x0 - B*a1c2*x4w0 - B*b1c2*y4w0 + C*b1c2*y4y0
				+ C*c1c2*w4y0 - C*c1a2*x4y0 - C*c1b2*y4y0 - C*c1c2*y4w0 + C*a1c2*y4x0;

			ee = -A*c1a2*w4x0 - A*c1b2*y4w0 - A*c1b2*w4y0 - A*a1b2*x4y0 + A*a1c2*x4w0
				+ A*b1c2*y4w0 + A*b1c2*w4y0 + A*b1a2*y4x0 - B*b1a2*x4x0 - B*b1b2*x4y0
				+ B*c1b2*x4w0 + B*a1b2*x4x0 + B*b1b2*y4x0 - B*b1c2*w4x0 - C*b1c2*x4y0
				+ C*c1c2*x4w0 + C*c1a2*x4x0 + C*c1b2*y4x0 - C*c1c2*w4x0 - C*a1c2*x4x0;

			EllipseAlgebraicParameters params = { aa,bb,cc,dd,ee,ff };
			return params;
		}
	};

	template<typename tFloat>
	struct EllipseParameters
	{
	public:
		/// <summary>	The x-coordinate of the center of the ellipse. </summary>
		tFloat x0;

		/// <summary>	The y-coordinate of the center of the ellipse. </summary>
		tFloat y0;

		/// <summary>	The length of the semi-minor axis. </summary>
		tFloat a;

		/// <summary>	The length of the semi-major axis. </summary>
		tFloat b;

		/// <summary>	The angle  between the x-axis and the major-axis. < / summary>
		tFloat theta;

		bool IsValid() const
		{
			return !std::isnan(this->x0);
		}

		static EllipseParameters<tFloat> Invalid()
		{
			return  EllipseParameters<tFloat>
			{
				std::numeric_limits<tFloat>::quiet_NaN(), std::numeric_limits<tFloat>::quiet_NaN(), std::numeric_limits<tFloat>::quiet_NaN(), std::numeric_limits<tFloat>::quiet_NaN(), std::numeric_limits<tFloat>::quiet_NaN()
			};
		}

		static EllipseParameters<tFloat> FromAlgebraicParameters(const EllipseAlgebraicParameters<tFloat>& algebraic)
		{
			if (!algebraic.IsEllipse())
			{
				return Invalid();
			}

			EllipseParameters<tFloat> p;
			tFloat s = 4 * algebraic.a*algebraic.c - algebraic.b*algebraic.b;
			tFloat sigma = 4 * (algebraic.c*algebraic.d*algebraic.d + algebraic.a*algebraic.e*algebraic.e - algebraic.b*algebraic.d*algebraic.e - algebraic.f*(4 * algebraic.a*algebraic.c - algebraic.b*algebraic.b)) / (s*s);

			tFloat num = (4 * algebraic.a*algebraic.c - algebraic.b*algebraic.b);
			p.x0 = (algebraic.b*algebraic.e - 2 * algebraic.c*algebraic.d) / num;
			p.y0 = (algebraic.b*algebraic.d - 2 * algebraic.a*algebraic.e) / num;

			s = sigma*algebraic.a - sigma*algebraic.c;
			s *= s;
			tFloat sigmabsquared = sigma*algebraic.b;
			sigmabsquared *= sigmabsquared;
			tFloat v1 = sigma*algebraic.a + sigma*algebraic.c + sqrt(s + sigmabsquared);
			tFloat v2 = sigma*algebraic.a + sigma*algebraic.c - sqrt(s + sigmabsquared);

			p.a = sqrt(abs((sigma*algebraic.a + sigma*algebraic.c + sqrt(s + sigmabsquared)) / 2));
			p.b = sqrt(abs((sigma*algebraic.a + sigma*algebraic.c - sqrt(s + sigmabsquared)) / 2));

			p.theta = atan2(algebraic.b, algebraic.a - algebraic.c) / 2;

			// that's the angle between the x-axis and the ellipse's major axis
			// all we have to do is to check what the major axis is...
			if (sigma > 0)
			{
				p.theta += M_PI_2;
			}

			return p;
		}
	};

}