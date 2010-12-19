/*******************************************************************************
 File:				matrix.inline.h

 Author: 			Gaspard Petit (gaspardpetit@gmail.com)
 Last Revision:		March 14, 2007

 This code may be reused without my permission as long as credits are given to
 the original author.  If you find bugs, please send me a note...
*******************************************************************************/
#ifndef __MK_GEOMETRY_matrix_INLINE__
#define __MK_GEOMETRY_matrix_INLINE__

//==============================================================================
//	EXTERNAL DECLARATIONS
//==============================================================================
#include "matrix.h"

#ifdef USE_matrixUTILS
#include "matrixUtils.h"
#endif // USE_matrixUTILS

#include <cstdio>
#include <cmath>

#include <cassert>
#define ASSERT assert

//==============================================================================
//	CLASS matrix
//==============================================================================
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT>::matrix()
{}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT>::matrix(const _T *data)
{
	for (int i = 0; i < count(); ++i)
		element[i] = data[i];
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT>::matrix(const matrix<_T, _WIDTH, _HEIGHT> &m)
{
	for (int i = 0; i < count(); ++i)
		element[i] = m.element[i];
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT> template<class _U>
matrix<_T, _WIDTH, _HEIGHT>& matrix<_T, _WIDTH, _HEIGHT>::operator = (const matrix<_U, _WIDTH, _HEIGHT> &m)
{
	for (int i = 0; i < count(); ++i)
		element[i] = m.element[i];

	return *this;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
const matrix<_T, _WIDTH, _HEIGHT>& matrix<_T, _WIDTH, _HEIGHT>::identitymatrix()
{
	static matrix<_T, _WIDTH, _HEIGHT> sIdentitymatrix = zeromatrix();
	static bool hasIdentitymatrix = false;
	if (!hasIdentitymatrix)
	{
		if (_WIDTH < _HEIGHT)
		{
			for (int i = 0; i < _WIDTH; ++i)
				sIdentitymatrix[i][i] = 1;
		}
		else
		{
			for (int i = 0; i < _HEIGHT; ++i)
				sIdentitymatrix[i][i] = 1;
		}

		hasIdentitymatrix = true;
	}

	return sIdentitymatrix;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
const matrix<_T, _WIDTH, _HEIGHT>& matrix<_T, _WIDTH, _HEIGHT>::zeromatrix()
{
	static matrix<_T, _WIDTH, _HEIGHT> sZeromatrix;
	static bool hasZeromatrix = false;
	if (!hasZeromatrix)
	{
		memset(sZeromatrix.ptr(), 0, sZeromatrix.size());
		hasZeromatrix = true;
	}

	return sZeromatrix;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
bool matrix<_T, _WIDTH, _HEIGHT>::operator == (const matrix &m) const
{
	return memcmp(element, m.element, size()) == 0;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
bool matrix<_T, _WIDTH, _HEIGHT>::operator != (const matrix &m) const
{
	return memcmp(element, m.element, size()) != 0;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
const _T* matrix<_T, _WIDTH, _HEIGHT>::operator [] (int row) const
{
	return &element[row*_WIDTH];
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
_T* matrix<_T, _WIDTH, _HEIGHT>::operator [] (int row)
{
	return &element[row*_WIDTH];
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
int matrix<_T, _WIDTH, _HEIGHT>::width() const
{
	return _WIDTH;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
int matrix<_T, _WIDTH, _HEIGHT>::height() const
{
	return _HEIGHT;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
const _T* matrix<_T, _WIDTH, _HEIGHT>::ptr() const
{
	return element;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
_T* matrix<_T, _WIDTH, _HEIGHT>::ptr()
{
	return element;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT> template<class _U>
matrix<_T, _WIDTH, _HEIGHT>& matrix<_T, _WIDTH, _HEIGHT>::copy(const _U *data)
{
	for (int i = 0; i < count(); ++i)
		element[i] = data[i];
	return *this;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::operator + (const matrix<_T, _WIDTH, _HEIGHT> &rhs) const
{
	matrix<_T, _WIDTH, _HEIGHT> m;

	for (int i = 0; i < count(); ++i)
		m.element[i] = element[i] + rhs.element[i];

	return m;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT>& matrix<_T, _WIDTH, _HEIGHT>::operator += (const matrix<_T, _WIDTH, _HEIGHT> &rhs)
{
	for (int i = 0; i < count(); ++i)
		element[i] += rhs.element[i];

	return *this;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::operator - (const matrix<_T, _WIDTH, _HEIGHT> &rhs) const
{
	matrix<_T, _WIDTH, _HEIGHT> m;

	for (int i = 0; i < count(); ++i)
		m.element[i] = element[i] - rhs.element[i];

	return m;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT>& matrix<_T, _WIDTH, _HEIGHT>::operator -= (const matrix<_T, _WIDTH, _HEIGHT> &rhs)
{
	for (int i = 0; i < count(); ++i)
		element[i] -= rhs.element[i];

	return *this;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT>& matrix<_T, _WIDTH, _HEIGHT>::operator - ()
{
	for (int i = 0; i < count(); ++i)
		element[i] = -element[i];

	return *this;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT> template<int _RHSWIDTH>
matrix<_T, _RHSWIDTH, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::operator * (const matrix<_T, _RHSWIDTH, _WIDTH> &rhs) const
{
	matrix<_T, _RHSWIDTH, _HEIGHT> m;

	int i, j, x;
    for (j = 0; j < _HEIGHT; ++j)
    {
    	for (i = 0; i < _RHSWIDTH; ++i)
        {
			m[j][i] = element[j*_WIDTH] * rhs[0][i];
        	for (x = 1; x < _WIDTH; ++x)
        	{
				m[j][i] += element[j*_WIDTH + x] * rhs[x][i];
			}
		}
	}
    return m;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::operator *= (const matrix<_T, _WIDTH, _HEIGHT> &rhs)
{
	(*this) = (*this) * rhs;
	return *this;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::operator * (const _T &rhs) const
{
	matrix<_T, _WIDTH, _HEIGHT> m;
	int i;
	for (i = 0; i < count(); ++i)
		m.element[i] = element[i] * rhs;
	return m;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT>& matrix<_T, _WIDTH, _HEIGHT>::operator *= (const _T &rhs)
{
	int i;
	for (i = 0; i < count(); ++i)
		element[i] *= rhs;
	return *this;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::operator / (const _T &rhs) const
{
	matrix<_T, _WIDTH, _HEIGHT> m;

	float oneOverRhs = 1.0f / rhs;

	int i;
	for (i = 0; i < count(); ++i)
		m.element[i] = element[i] * oneOverRhs;

	return m;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT>& matrix<_T, _WIDTH, _HEIGHT>::operator /= (const _T &rhs)
{
	float oneOverRhs = 1.0f / rhs;

	int i;
	for (i = 0; i < count(); ++i)
		element[i] *= oneOverRhs;
	return *this;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
void matrix<_T, _WIDTH, _HEIGHT>::transpose()
{
	*this = transposed();
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _HEIGHT, _WIDTH> matrix<_T, _WIDTH, _HEIGHT>::transposed() const
{
	matrix<_T, _HEIGHT, _WIDTH> m;

	for (int j = 0; j < _HEIGHT; ++j)
		for (int i = 0; i < _WIDTH; ++i)
			m.element[i*_HEIGHT+j] = element[j*_WIDTH+i];

	return m;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
std::string matrix<_T, _WIDTH, _HEIGHT>::serialize() const
{
	std::string str = "";
	char buf[2048];

	sprintf(buf, "{ "); str += buf;

	for (int j = 0; j < _HEIGHT; ++j)
	{
		sprintf(buf, "{ "); str += buf;
		for (int i = 0; i < _WIDTH; ++i)
		{
			sprintf(buf, "%.10f", element[j*_WIDTH+i]); str += buf;
			if (i < _WIDTH-1)
			{
				sprintf(buf, ", "); str += buf;
			}
		}
		sprintf(buf, " }\n"); str += buf;
	}
	sprintf(buf, " }"); str += buf;
	return str;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::deSerialize(const std::string &str)
{
	char buf[2048];
	strcpy(buf, str.c_str());
	matrix<_T, _WIDTH, _HEIGHT> m = matrix<_T, _WIDTH, _HEIGHT>::zeromatrix();

	char *value;
	value = strtok(buf, " \t\r\n{,}");

	int i = 0;
	while (value && i < _WIDTH*_HEIGHT)
	{
		double v;
		sscanf(value, "%lf", &v);
		m.element[i++] = v;
		value = strtok(NULL, " \t\r\n{,}");
	}

	return m;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
bool matrix<_T, _WIDTH, _HEIGHT>::hasNan() const
{
#ifndef WIN32
	for (int i = 0; i < count(); ++i)
		if (std::isnan(element[i]))	return true;
#else
#pragma message (__LOC__ "find the equivalent of isnan for windows...")
#endif

	return false;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
bool matrix<_T, _WIDTH, _HEIGHT>::hasInf() const
{
#ifndef WIN32
	for (int i = 0; i < count(); ++i)
		if (std::isinf(element[i]))	return true;
#else
#pragma message (__LOC__ "find the equivalent of isinf for windows...")
#endif

	return false;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT> operator * (int lhs, const matrix<_T, _WIDTH, _HEIGHT> &m)
{
	return m*lhs;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT> operator * (float lhs, const matrix<_T, _WIDTH, _HEIGHT> &m)
{
	return m*lhs;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT> operator * (double lhs, const matrix<_T, _WIDTH, _HEIGHT> &m)
{
	return m*lhs;
}

//------------------------------------------------------------------------------
//
template<class _T, int N>
static inline _T Determinant(const matrix<_T, N, N> &m)
{
	// not the most efficient way, but should work for now...
	_T determinant = 0;
	int factor = 1;
	for (int i = 0; i < N; ++i)
	{
		_T coeff = m[0][i];
		matrix<_T, N-1, N-1> subM;

		for (int x = 0; x < i; ++x)
			for (int y = 1; y < N; ++y)
				subM[y-1][x] = m[y][x];

		for (int x = i+1; x < N; ++x)
			for (int y = 1; y < N; ++y)
				subM[y-1][x-1] = m[y][x];


		determinant += coeff*factor*Determinant(subM);
		factor *= -1;
	}
	return determinant;
}

#ifndef WIN32

//------------------------------------------------------------------------------
//
template<class _T>
static inline _T Determinant(const matrix<_T, 1, 1> &m)
{
	return m[0][0];
}

#else

//------------------------------------------------------------------------------
//
template<>
static inline double Determinant(const matrix<double, 1, 1> &m)
{
	return m[0][0];
}

//------------------------------------------------------------------------------
//
template<>
static inline float Determinant(const matrix<float, 1, 1> &m)
{
	return m[0][0];
}

#endif

#ifdef USE_matrixUTILS
//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
void matrix<_T, _WIDTH, _HEIGHT>::inverse()
{
	*this = inversed();
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::inversed() const
{
	matrix<_T, _WIDTH, _HEIGHT> inverse;
	matrixUtils::luInverse(*this, &inverse, true);
	return inverse;
}
#endif // USE_matrixUTILS

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
size_t matrix<_T, _WIDTH, _HEIGHT>::size() const
{
	return count()*sizeof(_T);
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
int matrix<_T, _WIDTH, _HEIGHT>::count() const
{
	return _WIDTH*_HEIGHT;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
template <int _MW, int _MH>
matrix<_T, _WIDTH, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::submatrix(int r, int c, const matrix<_T, _MW, _MH> &m)
{
	ASSERT(_WIDTH + c <= _MW);
	ASSERT(_HEIGHT + r <= _MH);

	matrix<_T, _WIDTH, _HEIGHT> sub;
	for (int j = 0; j < _HEIGHT; ++j)
		for (int i = 0; i < _WIDTH; ++i)
			sub[j][i] = m[r+j][c+i];

	return sub;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
template <int _MW, int _MH>
matrix<_T, _WIDTH, _HEIGHT>& matrix<_T, _WIDTH, _HEIGHT>::copy(int r, int c, const matrix<_T, _MW, _MH> &m)
{
	for (int j = 0; j < _MH && (j+r) < _HEIGHT; ++j)
		for (int i = 0; i < _MW && (i+c) < _WIDTH; ++i)
			element[(j+r)*_WIDTH+(i+c)] = m[j][i];

	return *this;
}


//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT>& matrix<_T, _WIDTH, _HEIGHT>::copyRow(int index, const matrix<_T, _WIDTH, 1> &m)
{
	return copy(index, 0, m);
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT>& matrix<_T, _WIDTH, _HEIGHT>::copyCol(int index, const matrix<_T, 1, _HEIGHT> &m)
{
	return copy(0, index, m);
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, 1> matrix<_T, _WIDTH, _HEIGHT>::row(int index) const
{
	return matrix<_T, _WIDTH, 1>::submatrix(index, 0, *this);
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, 1, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::col(int index) const
{
	return matrix<_T, 1, _HEIGHT>::submatrix(0, index, *this);
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT+1> matrix<_T, _WIDTH, _HEIGHT>::addBackRow(const matrix<_T, _WIDTH, 1> &r ) const
{
	return addRow(_HEIGHT, r);
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH+1, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::addBackCol(const matrix<_T, 1, _HEIGHT> &c) const
{
	return addCol(_WIDTH, c);
}


//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT+1> matrix<_T, _WIDTH, _HEIGHT>::addFrontRow(const matrix<_T, _WIDTH, 1> &r ) const
{
	return addRow(0, r);
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH+1, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::addFrontCol(const matrix<_T, 1, _HEIGHT> &c) const
{
	return addCol(0, c);
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT+1> matrix<_T, _WIDTH, _HEIGHT>::addRow(int index, const matrix<_T, _WIDTH, 1> &r ) const
{
	matrix<_T, _WIDTH, _HEIGHT+1> m;
	int i = 0, ri = 0;

	for (; i < index; ++i)	m.copyRow(ri++, row(i));
	m.copyRow(ri++, r);
	for (; i < index; ++i)	m.copyRow(ri++, row(i));

	return m;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH+1, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::addCol(int index, const matrix<_T, 1, _HEIGHT> &c) const
{
	matrix<_T, _WIDTH+1, _HEIGHT> m;
	int i = 0, ci = 0;

	for (; i < index; ++i)	m.copyCol(ci++, col(i));
	m.copyCol(ci++, c);
	for (; i < index; ++i)	m.copyCol(ci++, col(i));

	return m;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT-1> matrix<_T, _WIDTH, _HEIGHT>::removeBackRow() const
{
	return matrix<_T, _WIDTH, _HEIGHT-1>::submatrix(0, 0, *this);
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH-1, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::removeBackCol() const
{
	return matrix<_T, _WIDTH-1, _HEIGHT>::submatrix(0, 0, *this);
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT-1> matrix<_T, _WIDTH, _HEIGHT>::removeFrontRow() const
{
	return matrix<_T, _WIDTH, _HEIGHT-1>::submatrix(1, 0, *this);
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH-1, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::removeFrontCol() const
{
	return matrix<_T, _WIDTH-1, _HEIGHT>::submatrix(0, 1, *this);
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH, _HEIGHT-1> matrix<_T, _WIDTH, _HEIGHT>::removeRow(int index ) const
{
	matrix<_T, _WIDTH, _HEIGHT-1> m;
	int i = 0, ri = 0;

	for (; i < index; ++i)	m.copyRow(ri++, row(i));
	++i; // skip one row
	for (; i < index; ++i)	m.copyRow(ri++, row(i));

	return m;
}

//------------------------------------------------------------------------------
//
template<class _T, int _WIDTH, int _HEIGHT>
matrix<_T, _WIDTH-1, _HEIGHT> matrix<_T, _WIDTH, _HEIGHT>::removeCol(int index) const
{
	matrix<_T, _WIDTH-1, _HEIGHT> m;
	int i = 0, ci = 0;

	for (; i < index; ++i)	m.copyCol(ci++, col(i));
	++i; // skip one column
	for (; i < index; ++i)	m.copyCol(ci++, col(i));

	return m;
}


#endif // __MK_GEOMETRY_matrix_INLINE__
