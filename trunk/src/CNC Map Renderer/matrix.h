/*******************************************************************************
 File:				matrix.h

 Author: 			Gaspard Petit (gaspardpetit@gmail.com)
 Last Revision:		March 14, 2007
 
 This code may be reused without my permission as long as credits are given to
 the original author.  If you find bugs, please send me a note...
*******************************************************************************/
#ifndef __MK_GEOMETRY_matrix__
#define __MK_GEOMETRY_matrix__

//==============================================================================
//	EXTERNAL DECLARATIONS
//==============================================================================
#include <string>

// uncomment if you have matrixUtils.h
//#define USE_matrixUTILS

//==============================================================================
//	CLASS matrix
//==============================================================================
template<class _T, int _WIDTH, int _HEIGHT>
class matrix
{
public:
	typedef _T Type;

public:
	matrix();
	matrix(const _T *data);
	matrix(const matrix<_T, _WIDTH, _HEIGHT> &m);

	template<class _U>
	matrix& operator = (const matrix<_U, _WIDTH, _HEIGHT> &m);
	
	static const matrix& identitymatrix();
	static const matrix& zeromatrix();

	template <int _MW, int _MH>
	static matrix submatrix(int r, int c, const matrix<_T, _MW, _MH> &m);

	bool operator == (const matrix &m) const;
	bool operator != (const matrix &m) const;

	const _T* operator [] (int row) const;
	_T* operator [] (int row);

	int width() const;
	int height() const;

	const _T* ptr() const;
	_T* ptr();

	template<class _U>
	matrix& copy(const _U *data);

	matrix operator + (const matrix &rhs) const;
	matrix& operator += (const matrix &rhs);
	matrix operator - (const matrix &rhs) const;
	matrix& operator -= (const matrix &rhs);
	matrix& operator - ();

	template<int _RHSWIDTH>
	matrix<_T, _RHSWIDTH, _HEIGHT> operator * (const matrix<_T, _RHSWIDTH, _WIDTH> &rhs) const;
	matrix operator *= (const matrix<_T, _WIDTH, _HEIGHT> &rhs);

	matrix operator * (const _T &rhs) const;
	matrix& operator *= (const _T &rhs);

	matrix operator / (const _T &rhs) const;
	matrix& operator /= (const _T &rhs);
	
	matrix<_T, _HEIGHT, _WIDTH> transposed() const;

	//	will work only if the matrix is square
	void transpose();

#ifdef USE_matrixUTILS
	void inverse();
	matrix inversed() const;
#endif // USE_matrixUTILS

	bool hasNan() const;
	bool hasInf() const;
	
	std::string serialize() const;
	static matrix deSerialize(const std::string &str);

	template<int _MW, int _MH>
	matrix<_T, _WIDTH, _HEIGHT>& copy(int r, int c, const matrix<_T, _MW, _MH> &m);

	matrix<_T, _WIDTH, _HEIGHT>& copyRow(int index, const matrix<_T, _WIDTH, 1> &m);
	matrix<_T, _WIDTH, _HEIGHT>& copyCol(int index, const matrix<_T, 1, _HEIGHT> &m);

	matrix<_T, _WIDTH, 1> row(int index) const;
	matrix<_T, 1, _HEIGHT> col(int index) const;

	matrix<_T, _WIDTH, _HEIGHT+1> addBackRow(const matrix<_T, _WIDTH, 1> &r ) const;
	matrix<_T, _WIDTH+1, _HEIGHT> addBackCol(const matrix<_T, 1, _HEIGHT> &c) const;

	matrix<_T, _WIDTH, _HEIGHT+1> addFrontRow(const matrix<_T, _WIDTH, 1> &r ) const;
	matrix<_T, _WIDTH+1, _HEIGHT> addFrontCol(const matrix<_T, 1, _HEIGHT> &c) const;

	matrix<_T, _WIDTH, _HEIGHT+1> addRow(int index, const matrix<_T, _WIDTH, 1> &r ) const;
	matrix<_T, _WIDTH+1, _HEIGHT> addCol(int index, const matrix<_T, 1, _HEIGHT> &c) const;

	matrix<_T, _WIDTH-1, _HEIGHT> removeBackCol() const;
	matrix<_T, _WIDTH-1, _HEIGHT> removeFrontCol() const;
	matrix<_T, _WIDTH-1, _HEIGHT> removeCol(int index) const;

	matrix<_T, _WIDTH, _HEIGHT-1> removeBackRow() const;
	matrix<_T, _WIDTH, _HEIGHT-1> removeFrontRow() const;
	matrix<_T, _WIDTH, _HEIGHT-1> removeRow(int index) const;

public:
	_T element[_WIDTH*_HEIGHT];

private:
	size_t size() const;
	int count() const;
};

//==============================================================================
//	TYPE DECLARATIONS
//==============================================================================
typedef matrix<float, 1, 1> matrix1f;
typedef matrix<float, 2, 2> matrix2f;
typedef matrix<float, 3, 3> matrix3f;
typedef matrix<float, 4, 4> matrix4f;
typedef matrix<double, 1, 1> matrix1d;
typedef matrix<double, 2, 2> matrix2d;
typedef matrix<double, 3, 3> matrix3d;
typedef matrix<double, 4, 4> matrix4d;

//==============================================================================
//	INLINED CODE
//==============================================================================
#include "matrix.inline.h"

#endif // __MK_GEOMETRY_matrix__
