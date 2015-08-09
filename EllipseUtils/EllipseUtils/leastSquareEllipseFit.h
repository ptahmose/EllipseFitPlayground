#pragma once

#include "ellipseParameters.h"
#include "inc_eigen.h"

namespace EllipseUtils
{
	template<typename tFloat>
	class LeastSquareEllipseFitter
	{
	public:
		class PointAccessorFromTwoVectors
		{
		private:
			const std::vector<tFloat>& pointsX;
			const std::vector<tFloat>& pointsY;
		public:
			PointAccessorFromTwoVectors(const std::vector<tFloat>& pointsX, const std::vector<tFloat>& pointsY)
				: pointsX(pointsX), pointsY(pointsY)
			{}

			size_t GetLength() const
			{
				return this->pointsX.size();
			}

			tFloat GetX(size_t index) const
			{
				return this->pointsX[index];
			}

			tFloat GetY(size_t index) const
			{
				return this->pointsY[index];
			}
		};

		class PointAccessorFromTwoArrays
		{
		private:
			const double* ptrX;
			const double* ptrY;
			size_t count;
		public:
			PointAccessorFromTwoArrays(const double* ptrX, const double* ptrY,size_t count)
				: ptrX(ptrX),ptrY(ptrY),count(count)
			{}


			size_t GetLength() const
			{
				return this->count;
			}

			tFloat GetX(size_t index) const
			{
				return this->ptrX[index];
			}

			tFloat GetY(size_t index) const
			{
				return this->ptrY[index];
			}
		};

		template <typename PointAccessor>
		static EllipseAlgebraicParameters<tFloat> Fit(const PointAccessor& ptAccessor)
		{
			auto numOfPoints = ptAccessor.GetLength();
			tFloat mx, my; tFloat minX, minY, maxX, maxY;
			CalcMeanMinMax([&](int index)->tFloat {return ptAccessor.GetX(index); }, ptAccessor.GetLength(), mx, minX, maxX);
			CalcMeanMinMax([&](int index)->tFloat {return ptAccessor.GetY(index); }, ptAccessor.GetLength(), my, minY, maxY);
			tFloat sx = (maxX - minX) / 2;
			tFloat sy = (maxY - minY) / 2;

			tFloat* designM = (tFloat*)malloc(numOfPoints * 6 * sizeof(tFloat));
			for (size_t i = 0; i < numOfPoints; ++i)
			{
				tFloat x = ptAccessor.GetX(i); tFloat y = ptAccessor.GetY(i);
				x = (x - mx) / sx;
				y = (y - my) / sy;
				designM[i * 6 + 0] = x*x;
				designM[i * 6 + 1] = x*y;
				designM[i * 6 + 2] = y*y;
				designM[i * 6 + 3] = x;
				designM[i * 6 + 4] = y;
				designM[i * 6 + 5] = 1;
			}

			tFloat* scatterM = (tFloat*)malloc(6 * 6 * sizeof(tFloat));
			for (int r = 0; r < 6; ++r)
			{
				for (int c = 0; c < 6; ++c)
				{
					tFloat v = 0;
					for (size_t k = 0; k < numOfPoints; ++k)
					{
						tFloat v1 = designM[k * 6 + r];
						tFloat v2 = designM[c + k * 6];
						v += v1*v2;
					}

					scatterM[r * 6 + c] = v;
				}
			}

			tFloat* tmpBtimestmpE = (tFloat*)malloc(3 * 3 * sizeof(tFloat));
			CalcTmpBtimesTmpE(scatterM + 3, 6 * sizeof(tFloat), scatterM + (3 * 6) + 3, 6 * sizeof(tFloat), tmpBtimestmpE);

			tFloat* testA = (tFloat*)malloc(3 * 3 * sizeof(tFloat));
			CalcTestA(scatterM, 6 * sizeof(tFloat), scatterM + 3, 6 * sizeof(tFloat), scatterM + (3 * 6) + 3, 6 * sizeof(tFloat), testA);

			Eigen::EigenSolver<Eigen::Matrix<tFloat, 3, 3>> eigenSolver;

			Eigen::Matrix<tFloat, 3, 3> m;
			m(0, 0) = testA[0];
			m(0, 1) = testA[1];
			m(0, 2) = testA[2];
			m(1, 0) = testA[3];
			m(1, 1) = testA[4];
			m(1, 2) = testA[5];
			m(2, 0) = testA[6];
			m(2, 1) = testA[7];
			m(2, 2) = testA[8];

			eigenSolver.compute(m, true);

			int indexPositiveEigenValue = -1;
			auto eigenval = eigenSolver.eigenvalues();
			for (int i = 0; i < 3; ++i)
			{
				if (eigenval[i].real() < 0)
				{
					indexPositiveEigenValue = i;
					break;
				}
			}

			tFloat A[6];
			auto eigenVecs = eigenSolver.eigenvectors();
			A[0] = eigenVecs(0, indexPositiveEigenValue).real();
			A[1] = eigenVecs(1, indexPositiveEigenValue).real();
			A[2] = eigenVecs(2, indexPositiveEigenValue).real();

			CalcLowerHalf(scatterM + 3, 6 * sizeof(tFloat), scatterM + (3 * 6) + 3, 6 * sizeof(tFloat), A, A + 3);

			EllipseAlgebraicParameters<tFloat> params;
			params.a = A[0] * sy*sy;
			params.b = A[1] * sx*sy;
			params.c = A[2] * sx*sx;
			params.d = -2 * A[0] * sy*sy*mx - A[1] * sx*sy*my + A[3] * sx*sy*sy;
			params.e = -A[1] * sx*sy*mx - 2 * A[2] * sx*sx*my + A[4] * sx*sx*sy;
			params.f = A[0] * sy*sy*mx*mx + A[1] * sx*sy*mx*my + A[2] * sx*sx*my*my
							- A[3] * sx*sy*sy*mx - A[4] * sx*sx*sy*my
							+ A[5] * sx*sx*sy*sy;
			return params;
		}

	private:
		static void CalcMeanMinMax(const std::function<tFloat(int)> getVal, size_t count, tFloat& mean, tFloat& min, tFloat& max)
		{
			min = (std::numeric_limits<tFloat>::max)();
			max = (std::numeric_limits<tFloat>::min)();
			mean = 0;

			for (size_t i = 0; i < count; ++i)
			{
				tFloat value = getVal(i);
				if (value < min)
				{
					min = value;
				}

				if (value > max)
				{
					max = value;
				}

				mean += value;
			}

			mean = mean / count;
		}

		static tFloat squared(tFloat f)
		{
			return f*f;
		}

		static void CalcTmpBtimesTmpE(const tFloat* pB, int strideB, const tFloat* pC, int strideC, tFloat* pDest)
		{
			/* -> Mathematica:
			{{b11, b12, b13}, {b21, b22, b23}, {b31, b32,
			b33}}.(Inverse[{{c11, c12, c13}, {c21, c22, c23}, {c31, c32,
			c33}}].Transpose[{{b11, b12, b13}, {b21, b22, b23}, {b31, b32,
			b33}}]) // Simplify // CForm
			*/

#define b(r,c) *((tFloat*)(((char*)pB)+(r-1)*strideB+(c-1)*sizeof(tFloat)))
#define c(r,c) *((tFloat*)(((char*)pC)+(r-1)*strideC+(c-1)*sizeof(tFloat)))


			pDest[0] =
				(squared(b(1, 3))*(-(c(1, 2)*c(2, 1)) + c(1, 1)*c(2, 2)) + b(1, 3)*(b(1, 1)*(-(c(1, 3)*c(2, 2)) + c(1, 2)*c(2, 3) - c(2, 2)*c(3, 1) + c(2, 1)*c(3, 2)) + b(1, 2)*(c(1, 3)*c(2, 1) + c(1, 2)*c(3, 1) - c(1, 1)*(c(2, 3) + c(3, 2)))) + squared(b(1, 2))*(-(c(1, 3)*c(3, 1)) + c(1, 1)*c(3, 3)) + b(1, 1)*b(1, 2)*(c(2, 3)*c(3, 1) + c(1, 3)*c(3, 2) - (c(1, 2) + c(2, 1))*c(3, 3)) + squared(b(1, 1))*(-(c(2, 3)*c(3, 2)) + c(2, 2)*c(3, 3))) / (-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3));

			pDest[1] =
				(b(1, 3) *(-b(2, 3)* c(1, 2)* c(2, 1) + b(2, 3)* c(1, 1)* c(2, 2) + b(2, 2) *c(1, 2) *c(3, 1) - b(2, 1)* c(2, 2)* c(3, 1) -
					b(2, 2)* c(1, 1) *c(3, 2) + b(2, 1)* c(2, 1) *c(3, 2)) +
					b(1, 2) *(b(2, 3)* c(1, 3) *c(2, 1) - b(2, 3)* c(1, 1)* c(2, 3) - b(2, 2)* c(1, 3)* c(3, 1) + b(2, 1)* c(2, 3)* c(3, 1) +
						b(2, 2)* c(1, 1)* c(3, 3) - b(2, 1) *c(2, 1)* c(3, 3)) +
					b(1, 1) *(-b(2, 3)* c(1, 3)* c(2, 2) + b(2, 3)* c(1, 2)* c(2, 3) + b(2, 2)* c(1, 3)* c(3, 2) - b(2, 1)* c(2, 3)* c(3, 2) -
						b(2, 2) *c(1, 2) *c(3, 3) + b(2, 1)* c(2, 2)* c(3, 3))) / (-c(1, 3) *c(2, 2)* c(3, 1) + c(1, 2) *c(2, 3) *c(3, 1) +
							c(1, 3)* c(2, 1) *c(3, 2) - c(1, 1)* c(2, 3) *c(3, 2) - c(1, 2)* c(2, 1)* c(3, 3) + c(1, 1)* c(2, 2)* c(3, 3));

			pDest[2] =
				(b(1, 3)*(-(b(3, 3)*c(1, 2)*c(2, 1)) + b(3, 3)*c(1, 1)*c(2, 2) + b(3, 2)*c(1, 2)*c(3, 1) - b(3, 1)*c(2, 2)*c(3, 1) - b(3, 2)*c(1, 1)*c(3, 2) + b(3, 1)*c(2, 1)*c(3, 2)) + b(1, 2)*(b(3, 3)*c(1, 3)*c(2, 1) - b(3, 3)*c(1, 1)*c(2, 3) - b(3, 2)*c(1, 3)*c(3, 1) + b(3, 1)*c(2, 3)*c(3, 1) + b(3, 2)*c(1, 1)*c(3, 3) - b(3, 1)*c(2, 1)*c(3, 3)) + b(1, 1)*(-(b(3, 3)*c(1, 3)*c(2, 2)) + b(3, 3)*c(1, 2)*c(2, 3) + b(3, 2)*c(1, 3)*c(3, 2) - b(3, 1)*c(2, 3)*c(3, 2) - b(3, 2)*c(1, 2)*c(3, 3) + b(3, 1)*c(2, 2)*c(3, 3))) / (-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3));

			pDest[3] = (b(1, 3)*(-(b(2, 3)*c(1, 2)*c(2, 1)) + b(2, 2)*c(1, 3)*c(2, 1) + b(2, 3)*c(1, 1)*c(2, 2) - b(2, 1)*c(1, 3)*c(2, 2) - b(2, 2)*c(1, 1)*c(2, 3) + b(2, 1)*c(1, 2)*c(2, 3)) + b(1, 2)*(b(2, 3)*c(1, 2)*c(3, 1) - b(2, 2)*c(1, 3)*c(3, 1) - b(2, 3)*c(1, 1)*c(3, 2) + b(2, 1)*c(1, 3)*c(3, 2) + b(2, 2)*c(1, 1)*c(3, 3) - b(2, 1)*c(1, 2)*c(3, 3)) + b(1, 1)*(-(b(2, 3)*c(2, 2)*c(3, 1)) + b(2, 2)*c(2, 3)*c(3, 1) + b(2, 3)*c(2, 1)*c(3, 2) - b(2, 1)*c(2, 3)*c(3, 2) - b(2, 2)*c(2, 1)*c(3, 3) + b(2, 1)*c(2, 2)*c(3, 3))) /
				(-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3));

			pDest[4] = (squared(b(2, 3))*(-(c(1, 2)*c(2, 1)) + c(1, 1)*c(2, 2)) + b(2, 3)*(b(2, 1)*(-(c(1, 3)*c(2, 2)) + c(1, 2)*c(2, 3) - c(2, 2)*c(3, 1) + c(2, 1)*c(3, 2)) + b(2, 2)*(c(1, 3)*c(2, 1) + c(1, 2)*c(3, 1) - c(1, 1)*(c(2, 3) + c(3, 2)))) + squared(b(2, 2))*(-(c(1, 3)*c(3, 1)) + c(1, 1)*c(3, 3)) + b(2, 1)*b(2, 2)*(c(2, 3)*c(3, 1) + c(1, 3)*c(3, 2) - (c(1, 2) + c(2, 1))*c(3, 3)) + squared(b(2, 1))*(-(c(2, 3)*c(3, 2)) + c(2, 2)*c(3, 3))) /
				(-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3));

			pDest[5] = (b(2, 3)*(-(b(3, 3)*c(1, 2)*c(2, 1)) + b(3, 3)*c(1, 1)*c(2, 2) + b(3, 2)*c(1, 2)*c(3, 1) - b(3, 1)*c(2, 2)*c(3, 1) - b(3, 2)*c(1, 1)*c(3, 2) + b(3, 1)*c(2, 1)*c(3, 2)) + b(2, 2)*(b(3, 3)*c(1, 3)*c(2, 1) - b(3, 3)*c(1, 1)*c(2, 3) - b(3, 2)*c(1, 3)*c(3, 1) + b(3, 1)*c(2, 3)*c(3, 1) + b(3, 2)*c(1, 1)*c(3, 3) - b(3, 1)*c(2, 1)*c(3, 3)) +
				b(2, 1)*(-(b(3, 3)*c(1, 3)*c(2, 2)) + b(3, 3)*c(1, 2)*c(2, 3) + b(3, 2)*c(1, 3)*c(3, 2) - b(3, 1)*c(2, 3)*c(3, 2) - b(3, 2)*c(1, 2)*c(3, 3) + b(3, 1)*c(2, 2)*c(3, 3))) / (-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3));

			pDest[6] = (b(1, 3)*(-(b(3, 3)*c(1, 2)*c(2, 1)) + b(3, 2)*c(1, 3)*c(2, 1) + b(3, 3)*c(1, 1)*c(2, 2) - b(3, 1)*c(1, 3)*c(2, 2) - b(3, 2)*c(1, 1)*c(2, 3) + b(3, 1)*c(1, 2)*c(2, 3)) + b(1, 2)*(b(3, 3)*c(1, 2)*c(3, 1) - b(3, 2)*c(1, 3)*c(3, 1) - b(3, 3)*c(1, 1)*c(3, 2) + b(3, 1)*c(1, 3)*c(3, 2) + b(3, 2)*c(1, 1)*c(3, 3) - b(3, 1)*c(1, 2)*c(3, 3)) + b(1, 1)*(-(b(3, 3)*c(2, 2)*c(3, 1)) + b(3, 2)*c(2, 3)*c(3, 1) + b(3, 3)*c(2, 1)*c(3, 2) - b(3, 1)*c(2, 3)*c(3, 2) - b(3, 2)*c(2, 1)*c(3, 3) + b(3, 1)*c(2, 2)*c(3, 3))) /
				(-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3));

			pDest[7] = (b(2, 3)*(-(b(3, 3)*c(1, 2)*c(2, 1)) + b(3, 2)*c(1, 3)*c(2, 1) + b(3, 3)*c(1, 1)*c(2, 2) - b(3, 1)*c(1, 3)*c(2, 2) - b(3, 2)*c(1, 1)*c(2, 3) + b(3, 1)*c(1, 2)*c(2, 3)) + b(2, 2)*(b(3, 3)*c(1, 2)*c(3, 1) - b(3, 2)*c(1, 3)*c(3, 1) - b(3, 3)*c(1, 1)*c(3, 2) + b(3, 1)*c(1, 3)*c(3, 2) + b(3, 2)*c(1, 1)*c(3, 3) - b(3, 1)*c(1, 2)*c(3, 3)) +
				b(2, 1)*(-(b(3, 3)*c(2, 2)*c(3, 1)) + b(3, 2)*c(2, 3)*c(3, 1) + b(3, 3)*c(2, 1)*c(3, 2) - b(3, 1)*c(2, 3)*c(3, 2) - b(3, 2)*c(2, 1)*c(3, 3) + b(3, 1)*c(2, 2)*c(3, 3))) / (-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3));

			pDest[8] = (squared(b(3, 3))*(-(c(1, 2)*c(2, 1)) + c(1, 1)*c(2, 2)) + b(3, 3)*(b(3, 1)*(-(c(1, 3)*c(2, 2)) + c(1, 2)*c(2, 3) - c(2, 2)*c(3, 1) + c(2, 1)*c(3, 2)) + b(3, 2)*(c(1, 3)*c(2, 1) + c(1, 2)*c(3, 1) - c(1, 1)*(c(2, 3) + c(3, 2)))) + squared(b(3, 2))*(-(c(1, 3)*c(3, 1)) + c(1, 1)*c(3, 3)) + b(3, 1)*b(3, 2)*(c(2, 3)*c(3, 1) + c(1, 3)*c(3, 2) - (c(1, 2) + c(2, 1))*c(3, 3)) + squared(b(3, 1))*(-(c(2, 3)*c(3, 2)) + c(2, 2)*c(3, 3))) / (-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3));
#undef b
#undef c
		}

		static void CalcTestA(const tFloat* pA, int strideA, const tFloat* pB, int strideB, const tFloat* pC, int strideC, tFloat* pDest)
		{
			/* Mathematica:

			{{0, 0, -(1/2)}, {0, 1, 0}, {-(1/2), 0,
			0}}.({{a11, a12, a13}, {a21, a22, a23}, {a31, a32,
			a33}} - {{b11, b12, b13}, {b21, b22, b23}, {b31, b32,
			b33}}.(Inverse[{{c11, c12, c13}, {c21, c22, c23}, {c31, c32,
			c33}}].Transpose[{{b11, b12, b13}, {b21, b22, b23}, {b31,
			b32, b33}}])) // Simplify // CForm
			*/
#define a(r,c) *((tFloat*)(((char*)pA)+(r-1)*strideA+(c-1)*sizeof(tFloat)))
#define b(r,c) *((tFloat*)(((char*)pB)+(r-1)*strideB+(c-1)*sizeof(tFloat)))
#define c(r,c) *((tFloat*)(((char*)pC)+(r-1)*strideC+(c-1)*sizeof(tFloat)))
			pDest[0] = (b(1, 3)*(-(b(3, 3)*c(1, 2)*c(2, 1)) + b(3, 2)*c(1, 3)*c(2, 1) + b(3, 3)*c(1, 1)*c(2, 2) - b(3, 1)*c(1, 3)*c(2, 2) - b(3, 2)*c(1, 1)*c(2, 3) + b(3, 1)*c(1, 2)*c(2, 3)) - b(1, 1)*b(3, 3)*c(2, 2)*c(3, 1) + a(3, 1)*c(1, 3)*c(2, 2)*c(3, 1) + b(1, 1)*b(3, 2)*c(2, 3)*c(3, 1) - a(3, 1)*c(1, 2)*c(2, 3)*c(3, 1) + b(1, 1)*b(3, 3)*c(2, 1)*c(3, 2) - a(3, 1)*c(1, 3)*c(2, 1)*c(3, 2) - b(1, 1)*b(3, 1)*c(2, 3)*c(3, 2) + a(3, 1)*c(1, 1)*c(2, 3)*c(3, 2) - b(1, 1)*b(3, 2)*c(2, 1)*c(3, 3) + a(3, 1)*c(1, 2)*c(2, 1)*c(3, 3) + b(1, 1)*b(3, 1)*c(2, 2)*c(3, 3) - a(3, 1)*c(1, 1)*c(2, 2)*c(3, 3) +
				b(1, 2)*(b(3, 3)*c(1, 2)*c(3, 1) - b(3, 2)*c(1, 3)*c(3, 1) - b(3, 3)*c(1, 1)*c(3, 2) + b(3, 1)*c(1, 3)*c(3, 2) + b(3, 2)*c(1, 1)*c(3, 3) - b(3, 1)*c(1, 2)*c(3, 3))) / (2.*(-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3)));

			pDest[1] = (b(2, 3)*(-(b(3, 3)*c(1, 2)*c(2, 1)) + b(3, 2)*c(1, 3)*c(2, 1) + b(3, 3)*c(1, 1)*c(2, 2) - b(3, 1)*c(1, 3)*c(2, 2) - b(3, 2)*c(1, 1)*c(2, 3) + b(3, 1)*c(1, 2)*c(2, 3)) - b(2, 1)*b(3, 3)*c(2, 2)*c(3, 1) + a(3, 2)*c(1, 3)*c(2, 2)*c(3, 1) + b(2, 1)*b(3, 2)*c(2, 3)*c(3, 1) - a(3, 2)*c(1, 2)*c(2, 3)*c(3, 1) + b(2, 1)*b(3, 3)*c(2, 1)*c(3, 2) - a(3, 2)*c(1, 3)*c(2, 1)*c(3, 2) - b(2, 1)*b(3, 1)*c(2, 3)*c(3, 2) + a(3, 2)*c(1, 1)*c(2, 3)*c(3, 2) - b(2, 1)*b(3, 2)*c(2, 1)*c(3, 3) + a(3, 2)*c(1, 2)*c(2, 1)*c(3, 3) + b(2, 1)*b(3, 1)*c(2, 2)*c(3, 3) - a(3, 2)*c(1, 1)*c(2, 2)*c(3, 3) +
				b(2, 2)*(b(3, 3)*c(1, 2)*c(3, 1) - b(3, 2)*c(1, 3)*c(3, 1) - b(3, 3)*c(1, 1)*c(3, 2) + b(3, 1)*c(1, 3)*c(3, 2) + b(3, 2)*c(1, 1)*c(3, 3) - b(3, 1)*c(1, 2)*c(3, 3))) / (2.*(-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3)));

			pDest[2] = (squared(b(3, 3))*(-(c(1, 2)*c(2, 1)) + c(1, 1)*c(2, 2)) + a(3, 3)*c(1, 3)*c(2, 2)*c(3, 1) - a(3, 3)*c(1, 2)*c(2, 3)*c(3, 1) - a(3, 3)*c(1, 3)*c(2, 1)*c(3, 2) - squared(b(3, 1))*c(2, 3)*c(3, 2) + a(3, 3)*c(1, 1)*c(2, 3)*c(3, 2) + b(3, 3)*(b(3, 1)*(-(c(1, 3)*c(2, 2)) + c(1, 2)*c(2, 3) - c(2, 2)*c(3, 1) + c(2, 1)*c(3, 2)) + b(3, 2)*(c(1, 3)*c(2, 1) + c(1, 2)*c(3, 1) - c(1, 1)*(c(2, 3) + c(3, 2)))) + a(3, 3)*c(1, 2)*c(2, 1)*c(3, 3) + squared(b(3, 1))*c(2, 2)*c(3, 3) - a(3, 3)*c(1, 1)*c(2, 2)*c(3, 3) + squared(b(3, 2))*(-(c(1, 3)*c(3, 1)) + c(1, 1)*c(3, 3)) +
				b(3, 1)*b(3, 2)*(c(2, 3)*c(3, 1) + c(1, 3)*c(3, 2) - (c(1, 2) + c(2, 1))*c(3, 3))) / (2.*(-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3)));

			pDest[3] = (b(1, 3)*(b(2, 3)*c(1, 2)*c(2, 1) - b(2, 2)*c(1, 3)*c(2, 1) - b(2, 3)*c(1, 1)*c(2, 2) + b(2, 1)*c(1, 3)*c(2, 2) + b(2, 2)*c(1, 1)*c(2, 3) - b(2, 1)*c(1, 2)*c(2, 3)) + b(1, 1)*b(2, 3)*c(2, 2)*c(3, 1) - a(2, 1)*c(1, 3)*c(2, 2)*c(3, 1) - b(1, 1)*b(2, 2)*c(2, 3)*c(3, 1) + a(2, 1)*c(1, 2)*c(2, 3)*c(3, 1) - b(1, 1)*b(2, 3)*c(2, 1)*c(3, 2) + a(2, 1)*c(1, 3)*c(2, 1)*c(3, 2) + b(1, 1)*b(2, 1)*c(2, 3)*c(3, 2) - a(2, 1)*c(1, 1)*c(2, 3)*c(3, 2) + b(1, 1)*b(2, 2)*c(2, 1)*c(3, 3) - a(2, 1)*c(1, 2)*c(2, 1)*c(3, 3) - b(1, 1)*b(2, 1)*c(2, 2)*c(3, 3) + a(2, 1)*c(1, 1)*c(2, 2)*c(3, 3) +
				b(1, 2)*(-(b(2, 3)*c(1, 2)*c(3, 1)) + b(2, 2)*c(1, 3)*c(3, 1) + b(2, 3)*c(1, 1)*c(3, 2) - b(2, 1)*c(1, 3)*c(3, 2) - b(2, 2)*c(1, 1)*c(3, 3) + b(2, 1)*c(1, 2)*c(3, 3))) / (-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3));

			pDest[4] = (squared(b(2, 3))*(-(c(1, 2)*c(2, 1)) + c(1, 1)*c(2, 2)) + a(2, 2)*c(1, 3)*c(2, 2)*c(3, 1) - a(2, 2)*c(1, 2)*c(2, 3)*c(3, 1) - a(2, 2)*c(1, 3)*c(2, 1)*c(3, 2) - squared(b(2, 1))*c(2, 3)*c(3, 2) + a(2, 2)*c(1, 1)*c(2, 3)*c(3, 2) + b(2, 3)*(b(2, 1)*(-(c(1, 3)*c(2, 2)) + c(1, 2)*c(2, 3) - c(2, 2)*c(3, 1) + c(2, 1)*c(3, 2)) + b(2, 2)*(c(1, 3)*c(2, 1) + c(1, 2)*c(3, 1) - c(1, 1)*(c(2, 3) + c(3, 2)))) + a(2, 2)*c(1, 2)*c(2, 1)*c(3, 3) + squared(b(2, 1))*c(2, 2)*c(3, 3) - a(2, 2)*c(1, 1)*c(2, 2)*c(3, 3) + squared(b(2, 2))*(-(c(1, 3)*c(3, 1)) + c(1, 1)*c(3, 3)) +
				b(2, 1)*b(2, 2)*(c(2, 3)*c(3, 1) + c(1, 3)*c(3, 2) - (c(1, 2) + c(2, 1))*c(3, 3))) / (c(1, 3)*c(2, 2)*c(3, 1) - c(1, 2)*c(2, 3)*c(3, 1) - c(1, 3)*c(2, 1)*c(3, 2) + c(1, 1)*c(2, 3)*c(3, 2) + c(1, 2)*c(2, 1)*c(3, 3) - c(1, 1)*c(2, 2)*c(3, 3));

			pDest[5] = (b(2, 1)*b(3, 3)*c(1, 3)*c(2, 2) - b(2, 1)*b(3, 3)*c(1, 2)*c(2, 3) - a(2, 3)*c(1, 3)*c(2, 2)*c(3, 1) + a(2, 3)*c(1, 2)*c(2, 3)*c(3, 1) - b(2, 1)*b(3, 2)*c(1, 3)*c(3, 2) + a(2, 3)*c(1, 3)*c(2, 1)*c(3, 2) + b(2, 1)*b(3, 1)*c(2, 3)*c(3, 2) - a(2, 3)*c(1, 1)*c(2, 3)*c(3, 2) +
				b(2, 3)*(b(3, 3)*c(1, 2)*c(2, 1) - b(3, 3)*c(1, 1)*c(2, 2) - b(3, 2)*c(1, 2)*c(3, 1) + b(3, 1)*c(2, 2)*c(3, 1) + b(3, 2)*c(1, 1)*c(3, 2) - b(3, 1)*c(2, 1)*c(3, 2)) + b(2, 1)*b(3, 2)*c(1, 2)*c(3, 3) - a(2, 3)*c(1, 2)*c(2, 1)*c(3, 3) - b(2, 1)*b(3, 1)*c(2, 2)*c(3, 3) + a(2, 3)*c(1, 1)*c(2, 2)*c(3, 3) + b(2, 2)*(-(b(3, 3)*c(1, 3)*c(2, 1)) + b(3, 3)*c(1, 1)*c(2, 3) + b(3, 2)*c(1, 3)*c(3, 1) - b(3, 1)*c(2, 3)*c(3, 1) - b(3, 2)*c(1, 1)*c(3, 3) + b(3, 1)*c(2, 1)*c(3, 3))) / (-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3));

			pDest[6] = (squared(b(1, 3))*(-(c(1, 2)*c(2, 1)) + c(1, 1)*c(2, 2)) + a(1, 1)*c(1, 3)*c(2, 2)*c(3, 1) - a(1, 1)*c(1, 2)*c(2, 3)*c(3, 1) - a(1, 1)*c(1, 3)*c(2, 1)*c(3, 2) - squared(b(1, 1))*c(2, 3)*c(3, 2) + a(1, 1)*c(1, 1)*c(2, 3)*c(3, 2) + b(1, 3)*(b(1, 1)*(-(c(1, 3)*c(2, 2)) + c(1, 2)*c(2, 3) - c(2, 2)*c(3, 1) + c(2, 1)*c(3, 2)) + b(1, 2)*(c(1, 3)*c(2, 1) + c(1, 2)*c(3, 1) - c(1, 1)*(c(2, 3) + c(3, 2)))) + a(1, 1)*c(1, 2)*c(2, 1)*c(3, 3) + squared(b(1, 1))*c(2, 2)*c(3, 3) - a(1, 1)*c(1, 1)*c(2, 2)*c(3, 3) + squared(b(1, 2))*(-(c(1, 3)*c(3, 1)) + c(1, 1)*c(3, 3)) +
				b(1, 1)*b(1, 2)*(c(2, 3)*c(3, 1) + c(1, 3)*c(3, 2) - (c(1, 2) + c(2, 1))*c(3, 3))) / (2.*(-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3)));

			pDest[7] = (-(b(1, 1)*b(2, 3)*c(1, 3)*c(2, 2)) + b(1, 1)*b(2, 3)*c(1, 2)*c(2, 3) + a(1, 2)*c(1, 3)*c(2, 2)*c(3, 1) - a(1, 2)*c(1, 2)*c(2, 3)*c(3, 1) + b(1, 1)*b(2, 2)*c(1, 3)*c(3, 2) - a(1, 2)*c(1, 3)*c(2, 1)*c(3, 2) - b(1, 1)*b(2, 1)*c(2, 3)*c(3, 2) + a(1, 2)*c(1, 1)*c(2, 3)*c(3, 2) +
				b(1, 3)*(-(b(2, 3)*c(1, 2)*c(2, 1)) + b(2, 3)*c(1, 1)*c(2, 2) + b(2, 2)*c(1, 2)*c(3, 1) - b(2, 1)*c(2, 2)*c(3, 1) - b(2, 2)*c(1, 1)*c(3, 2) + b(2, 1)*c(2, 1)*c(3, 2)) - b(1, 1)*b(2, 2)*c(1, 2)*c(3, 3) + a(1, 2)*c(1, 2)*c(2, 1)*c(3, 3) + b(1, 1)*b(2, 1)*c(2, 2)*c(3, 3) - a(1, 2)*c(1, 1)*c(2, 2)*c(3, 3) + b(1, 2)*(b(2, 3)*c(1, 3)*c(2, 1) - b(2, 3)*c(1, 1)*c(2, 3) - b(2, 2)*c(1, 3)*c(3, 1) + b(2, 1)*c(2, 3)*c(3, 1) + b(2, 2)*c(1, 1)*c(3, 3) - b(2, 1)*c(2, 1)*c(3, 3))) / (2.*(-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3)));

			pDest[8] = (-(b(1, 1)*b(3, 3)*c(1, 3)*c(2, 2)) + b(1, 1)*b(3, 3)*c(1, 2)*c(2, 3) + a(1, 3)*c(1, 3)*c(2, 2)*c(3, 1) - a(1, 3)*c(1, 2)*c(2, 3)*c(3, 1) + b(1, 1)*b(3, 2)*c(1, 3)*c(3, 2) - a(1, 3)*c(1, 3)*c(2, 1)*c(3, 2) - b(1, 1)*b(3, 1)*c(2, 3)*c(3, 2) + a(1, 3)*c(1, 1)*c(2, 3)*c(3, 2) + b(1, 3)*(-(b(3, 3)*c(1, 2)*c(2, 1)) + b(3, 3)*c(1, 1)*c(2, 2) + b(3, 2)*c(1, 2)*c(3, 1) - b(3, 1)*c(2, 2)*c(3, 1) - b(3, 2)*c(1, 1)*c(3, 2) + b(3, 1)*c(2, 1)*c(3, 2)) - b(1, 1)*b(3, 2)*c(1, 2)*c(3, 3) + a(1, 3)*c(1, 2)*c(2, 1)*c(3, 3) + b(1, 1)*b(3, 1)*c(2, 2)*c(3, 3) - a(1, 3)*c(1, 1)*c(2, 2)*c(3, 3) +
				b(1, 2)*(b(3, 3)*c(1, 3)*c(2, 1) - b(3, 3)*c(1, 1)*c(2, 3) - b(3, 2)*c(1, 3)*c(3, 1) + b(3, 1)*c(2, 3)*c(3, 1) + b(3, 2)*c(1, 1)*c(3, 3) - b(3, 1)*c(2, 1)*c(3, 3))) / (2.*(-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3)));

#undef c
#undef b
#undef a
		}
		
		static void CalcLowerHalf(const tFloat* pB, int strideB, const tFloat* pC, int strideC, const tFloat* ptrAUpperHalf, tFloat* ptrDest)
		{
#define b(r,c) *((tFloat*)(((char*)pB)+(r-1)*strideB+(c-1)*sizeof(tFloat)))
#define c(r,c) *((tFloat*)(((char*)pC)+(r-1)*strideC+(c-1)*sizeof(tFloat)))

			tFloat a1 = ptrAUpperHalf[0]; tFloat a2 = ptrAUpperHalf[1]; tFloat a3 = ptrAUpperHalf[2];
			ptrDest[0] = (a1*(b(1, 3)*c(1, 3)*c(2, 2) - b(1, 3)*c(1, 2)*c(2, 3) - b(1, 2)*c(1, 3)*c(3, 2) + b(1, 1)*c(2, 3)*c(3, 2) + b(1, 2)*c(1, 2)*c(3, 3) - b(1, 1)*c(2, 2)*c(3, 3)) + a2*(b(2, 3)*c(1, 3)*c(2, 2) - b(2, 3)*c(1, 2)*c(2, 3) - b(2, 2)*c(1, 3)*c(3, 2) + b(2, 1)*c(2, 3)*c(3, 2) + b(2, 2)*c(1, 2)*c(3, 3) - b(2, 1)*c(2, 2)*c(3, 3)) + a3*(b(3, 3)*c(1, 3)*c(2, 2) - b(3, 3)*c(1, 2)*c(2, 3) - b(3, 2)*c(1, 3)*c(3, 2) + b(3, 1)*c(2, 3)*c(3, 2) + b(3, 2)*c(1, 2)*c(3, 3) - b(3, 1)*c(2, 2)*c(3, 3))) / (-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3));
			ptrDest[1] = (a1*(-(b(1, 3)*c(1, 3)*c(2, 1)) + b(1, 3)*c(1, 1)*c(2, 3) + b(1, 2)*c(1, 3)*c(3, 1) - b(1, 1)*c(2, 3)*c(3, 1) - b(1, 2)*c(1, 1)*c(3, 3) + b(1, 1)*c(2, 1)*c(3, 3)) + a2*(-(b(2, 3)*c(1, 3)*c(2, 1)) + b(2, 3)*c(1, 1)*c(2, 3) + b(2, 2)*c(1, 3)*c(3, 1) - b(2, 1)*c(2, 3)*c(3, 1) - b(2, 2)*c(1, 1)*c(3, 3) + b(2, 1)*c(2, 1)*c(3, 3)) + a3*(-(b(3, 3)*c(1, 3)*c(2, 1)) + b(3, 3)*c(1, 1)*c(2, 3) + b(3, 2)*c(1, 3)*c(3, 1) - b(3, 1)*c(2, 3)*c(3, 1) - b(3, 2)*c(1, 1)*c(3, 3) + b(3, 1)*c(2, 1)*c(3, 3))) / (-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3));
			ptrDest[2] = (a1*(b(1, 3)*c(1, 2)*c(2, 1) - b(1, 3)*c(1, 1)*c(2, 2) - b(1, 2)*c(1, 2)*c(3, 1) + b(1, 1)*c(2, 2)*c(3, 1) + b(1, 2)*c(1, 1)*c(3, 2) - b(1, 1)*c(2, 1)*c(3, 2)) + a2*(b(2, 3)*c(1, 2)*c(2, 1) - b(2, 3)*c(1, 1)*c(2, 2) - b(2, 2)*c(1, 2)*c(3, 1) + b(2, 1)*c(2, 2)*c(3, 1) + b(2, 2)*c(1, 1)*c(3, 2) - b(2, 1)*c(2, 1)*c(3, 2)) + a3*(b(3, 3)*c(1, 2)*c(2, 1) - b(3, 3)*c(1, 1)*c(2, 2) - b(3, 2)*c(1, 2)*c(3, 1) + b(3, 1)*c(2, 2)*c(3, 1) + b(3, 2)*c(1, 1)*c(3, 2) - b(3, 1)*c(2, 1)*c(3, 2))) / (-(c(1, 3)*c(2, 2)*c(3, 1)) + c(1, 2)*c(2, 3)*c(3, 1) + c(1, 3)*c(2, 1)*c(3, 2) - c(1, 1)*c(2, 3)*c(3, 2) - c(1, 2)*c(2, 1)*c(3, 3) + c(1, 1)*c(2, 2)*c(3, 3));
#undef b
#undef c
		}
	};
}