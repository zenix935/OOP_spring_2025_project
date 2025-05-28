#pragma once
#ifndef MATRIX_H
#define MATRIX_H
#include <vector>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <numeric>
#include "Complex.h"
#include "Exeptions.h"
using namespace std;
template <typename T>
class Matrix
{
private:
	vector<vector<T>> data;
	size_t rows;
	size_t cols;
public:
	Matrix(size_t r,size_t c) : rows(r),cols(c)
	{
		data.resize(rows,vector<T>(cols,T(0.0)));
	}
	size_t getRows() const { return rows; }
	size_t getCols() const { return cols; }
	void set(size_t r,size_t c,const T& value)
	{
		if(r<rows&&c<cols)
			data[r][c]=value;
		else
			throw MatrixException("Index out of bounds");
	}	
	void add(size_t r,size_t c,const T& value)
	{
		if(r<rows&&c<cols)
			data[r][c]+=value;
		else
			throw MatrixException("Index out of bounds");
	}
	void resize(size_t newRows,size_t newCols)
	{
		data,resize(newRows);
		for(size_t i=0;i<newRows;i++)
			data[i].resize(newCols,T(0.0));
		rows=newRows;
		cols=newCols;
	}
	void print(const string& title="Matrix") const
	{
		cout<<title<<" ("<<rows<<"x"<<cols<<"):"<<endl;
		for(const auto& row:data)
		{
			for(const auto& elem:row)
				cout<<setw(15)<<elem<<" ";
			cout<<endl;
		}
	}
	//Gaussian Elimination solver for Ax=b
	// Returns the solution vector x
	vector<T> solveGuassianElimination(const vector<T>& b) const
	{
		if(rows!=cols||rows!=b.size())
			throw MatrixException("Matrix dimensions do not match for Gaussian elimination");
		size_t n=rows;
		vector<vector<T>> augmentedMatrix=data;
		for(size_t i=0;i<n;i++)
			augmentedMatrix[i].push_back(b[i]);
		for(size_t k=0; k<n; ++k)
		{
			// Find pivot row (partial pivoting)
			size_t pivotRow=k;
			// Use magnitude for pivoting for complex numbers
			for(size_t i=k+1;i<n;i++)
				if(augmentedMatrix[i][k].mag()>augmentedMatrix[pivotRow][k].mag())
					pivotRow=i;
			if(pivotRow!=k)
				std::swap(augmentedMatrix[k],augmentedMatrix[pivotRow]);
			// Check for singular matrix (using magnitude for complex numbers)
			if(augmentedMatrix[k][k].mag()<1e-9) // Using a small epsilon for zero check
				throw CircuitError("Matrix is singular or ill-conditioned. Cannot solve with Gaussian Elimination.");
			// Eliminate
			for(size_t i=k+1;i<n;i++)
			{
				T factor=augmentedMatrix[i][k]/augmentedMatrix[k][k];
				for(size_t j=k;j<=n;j++) // Iterate up to n (including b column)
					augmentedMatrix[i][j]-=factor*augmentedMatrix[k][j];
			}
		}
		// Backward Substitution
		std::vector<T> x(n);
		for(int i=n-1;i>=0;i--)
		{
			T sum=T(0.0);
			for(size_t j=i+1;j<n;j++)
				sum+=augmentedMatrix[i][j]*x[j];
			x[i]=(augmentedMatrix[i][n]-sum)/augmentedMatrix[i][i];
		}
		return x;
	}
};
#endif
