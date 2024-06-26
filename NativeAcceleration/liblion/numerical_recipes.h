/***************************************************************************
 *
 * Author: "Sjors H.W. Scheres"
 * MRC Laboratory of Molecular Biology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This complete copyright notice must be included in any revised version of the
 * source code. Additional authorship citations may be added, but existing
 * author citations must be preserved.
 ***************************************************************************/
/***************************************************************************
 *
 * Authors:     Carlos Oscar S. Sorzano (coss@cnb.csic.es)
 *
 * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/
#ifndef _NUMERICAL_HH
#   define _NUMERICAL_HH

#include <math.h>
#include "memory.h"
#include "macros.h"
#include "error.h"


namespace relion
{
	/*****************************************************************************/
	/* Variable and prototype definitions for the Numerical Core                 */
	/*****************************************************************************/

	//@defgroup NumericalRecipes Functions from the Numerical Recipes
	//@ingroup DataLibrary
	//@{

	// Utilities --------------------------------------------------------------
	void nrerror(const char error_text[]);

	// Bessel functions --------------------------------------------------------
	DOUBLE bessj0(DOUBLE x);
	DOUBLE bessj3_5(DOUBLE x);
	DOUBLE bessj1_5(DOUBLE x);

	DOUBLE bessi0(DOUBLE x);
	DOUBLE bessi1(DOUBLE x);
	DOUBLE bessi0_5(DOUBLE x);
	DOUBLE bessi1_5(DOUBLE x);
	DOUBLE bessi2(DOUBLE x);
	DOUBLE bessi3(DOUBLE x);
	DOUBLE bessi2_5(DOUBLE x);
	DOUBLE bessi3_5(DOUBLE x);
	DOUBLE bessi4(DOUBLE x);

	// Special functions -------------------------------------------------------
	DOUBLE gammln(DOUBLE xx);
	DOUBLE gammp(DOUBLE a, DOUBLE x);
	DOUBLE betacf(DOUBLE a, DOUBLE b, DOUBLE x);
	DOUBLE betai(DOUBLE a, DOUBLE b, DOUBLE x);

	// Singular value descomposition of matrix a (numerical recipes, chapter 2-6 for details)
	void svdcmp(DOUBLE *a, int m, int n, DOUBLE *w, DOUBLE *v);
	void svbksb(DOUBLE *u, DOUBLE *w, DOUBLE *v, int m, int n, DOUBLE *b, DOUBLE *x);

	// Optimization ------------------------------------------------------------
	void powell(DOUBLE *p, DOUBLE *xi, int n, DOUBLE ftol, int &iter,
		DOUBLE &fret, DOUBLE(*func)(DOUBLE *, void *), void *prm,
		bool show);

	// Working with matrices ---------------------------------------------------
	// LU decomposition
#define TINY 1.0e-20;
	/* Chapter 2 Section 3: LU DECOMPOSITION */
	template <class T>
	void ludcmp(T *a, int n, int *indx, T *d)
	{
		int i, imax, j, k;
		T big, dum, sum, temp;
		T *vv;

		ask_Tvector(vv, 1, n);
		*d = (T)1.0;
		for (i = 1; i <= n; i++)
		{
			big = (T)0.0;
			for (j = 1; j <= n; j++)
				if ((temp = (T)fabs((DOUBLE)a[i*n + j])) > big)
					big = temp;
			if (big == (T)0.0)
				nrerror("Singular matrix in routine LUDCMP");
			vv[i] = (T)1.0 / big;
		}
		for (j = 1; j <= n; j++)
		{
			for (i = 1; i < j; i++)
			{
				sum = a[i*n + j];
				for (k = 1; k < i; k++)
					sum -= a[i*n + k] * a[k*n + j];
				a[i*n + j] = sum;
			}
			big = (T)0.0;
			for (i = j; i <= n; i++)
			{
				sum = a[i*n + j];
				for (k = 1; k < j; k++)
					sum -= a[i*n + k] * a[k*n + j];
				a[i*n + j] = sum;
				if ((dum = vv[i] * (T)fabs((DOUBLE)sum)) >= big)
				{
					big = dum;
					imax = i;
				}
			}
			if (j != imax)
			{
				for (k = 1; k <= n; k++)
				{
					dum = a[imax*n + k];
					a[imax*n + k] = a[j*n + k];
					a[j*n + k] = dum;
				}
				*d = -(*d);
				vv[imax] = vv[j];
			}
			indx[j] = imax;
			if (a[j*n + j] == 0.0)
				a[j*n + j] = (T)TINY;
			if (j != n)
			{
				dum = (T)1.0 / (a[j*n + j]);
				for (i = j + 1; i <= n; i++)
					a[i*n + j] *= dum;
			}
		}
		free_Tvector(vv, 1, n);
	}
#undef TINY

	// Solve Ax=b
	/* Chapter 2 Section 3: LU BACKWARD-FORWARD SUBSTITUTION */
	template <class T>
	void lubksb(T *a, int n, int *indx, T b[])
	{
		int i, ii = 0, ip, j;
		T sum;

		for (i = 1; i <= n; i++)
		{
			ip = indx[i];
			sum = b[ip];
			b[ip] = b[i];
			if (ii)
				for (j = ii; j <= i - 1; j++)
					sum -= a[i*n + j] * b[j];
			else if (sum)
				ii = i;
			b[i] = sum;
		}
		for (i = n; i >= 1; i--)
		{
			sum = b[i];
			for (j = i + 1; j <= n; j++)
				sum -= a[i*n + j] * b[j];
			b[i] = sum / a[i*n + i];
		}
	}

	/* Chapter 2, Section 1. Gauss-Jordan equation system resolution ----------- */
	// Solve Ax=b (b=matrix)
	template <class T>
	void gaussj(T *a, int n, T *b, int m)
	{
		T temp;
		int *indxc, *indxr, *ipiv;
		int i, icol, irow, j, k, l, ll;
		T big, dum;
		DOUBLE pivinv;

		ask_Tvector(indxc, 1, n);
		ask_Tvector(indxr, 1, n);
		ask_Tvector(ipiv, 1, n);
		for (j = 1; j <= n; j++)
			ipiv[j] = 0;
		for (i = 1; i <= n; i++)
		{
			big = (T)0;
			for (j = 1; j <= n; j++)
				if (ipiv[j] != 1)
					for (k = 1; k <= n; k++)
					{
						if (ipiv[k] == 0)
						{
							if (fabs((DOUBLE)a[j*n + k]) >= (DOUBLE)big)
							{
								big = ABS(a[j*n + k]);
								irow = j;
								icol = k;
							}
						}
						else if (ipiv[k] > 1)
							nrerror("GAUSSJ: Singular Matrix-1");
					}
			++(ipiv[icol]);
			if (irow != icol)
			{
				for (l = 1; l <= n; l++)
					SWAP(a[irow*n + l], a[icol*n + l], temp)
					for (l = 1; l <= m; l++)
						SWAP(b[irow*n + l], b[icol*n + l], temp)
			}
			indxr[i] = irow;
			indxc[i] = icol;
			if (a[icol*n + icol] == 0.0)
				nrerror("GAUSSJ: Singular Matrix-2");
			pivinv = 1.0f / a[icol*n + icol];
			a[icol*n + icol] = (T)1;
			for (l = 1; l <= n; l++)
				a[icol*n + l] = (T)(pivinv * a[icol*n + l]);
			for (l = 1; l <= m; l++)
				b[icol*n + l] = (T)(pivinv * b[icol*n + l]);
			for (ll = 1; ll <= n; ll++)
				if (ll != icol)
				{
					dum = a[ll*n + icol];
					a[ll*n + icol] = (T)0;
					for (l = 1; l <= n; l++)
						a[ll*n + l] -= a[icol*n + l] * dum;
					for (l = 1; l <= m; l++)
						b[ll*n + l] -= b[icol*n + l] * dum;
				}
		}
		for (l = n; l >= 1; l--)
		{
			if (indxr[l] != indxc[l])
				for (k = 1; k <= n; k++)
					SWAP(a[k*n + indxr[l]], a[k*n + indxc[l]], temp);
		}
		free_Tvector(ipiv, 1, n);
		free_Tvector(indxr, 1, n);
		free_Tvector(indxc, 1, n);
	}

}
#endif
