#ifndef MATRIX_H
#define MATRIX_H
#include <vector>
#include <iostream>
#include <iomanip> 
#include <cmath>   
#include <numeric> 
#include "Complex.h"      
#include "Exceptions.h"   
// Helper function to get magnitude, works for primitive numeric types and Complex
template <typename T>
double get_magnitude(const T& val) { return std::fabs(val); }
// Specialization for Complex numbers to use their .mag() method
inline double get_magnitude(const Complex& val) { return val.mag(); }
// The Matrix class is templated to work with different types (double or Complex)
template <typename T>
class Matrix 
{
private:
    std::vector<std::vector<T>> data;
    size_t rows;
    size_t cols;

public:
    Matrix(size_t r,size_t c) : rows(r),cols(c) 
    {
        data.resize(rows,std::vector<T>(cols,T(0.0)));
    }
    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }
    void set(size_t r,size_t c,const T& value) 
    {
        if(r<rows&&c<cols)
            data[r][c]=value;
        else
            std::cerr<<"Error: Matrix index out of bounds ("<<r<<", "<<c<<")\n";
    }
    void add(size_t r,size_t c,const T& value) 
    {
        if(r<rows&&c<cols)
            data[r][c]+=value;
        else
            std::cerr<<"Error: Matrix index out of bounds ("<<r<<", "<<c<<")\n";
    }
    T get(size_t r,size_t c) const 
    {
        if(r<rows&&c<cols)
            return data[r][c];
        else 
        {
            std::cerr<<"Error: Matrix index out of bounds ("<<r<<", "<<c<<")\n";
            return T(0.0);
        }
    }
    // Resize the matrix (retains existing data if new size is smaller/same)
    void resize(size_t newRows,size_t newCols) 
    {
        data.resize(newRows);
        for(size_t i=0; i<newRows; ++i)
            data[i].resize(newCols,T(0.0));
        rows=newRows;
        cols=newCols;
    }
    void print(const std::string& title="Matrix") const 
    {
        std::cout<<title<<" ("<<rows<<"x"<<cols<<"):\n";
        for(size_t i=0; i<rows; ++i) 
        {
            for(size_t j=0; j<cols; ++j) 
                std::cout<<std::setw(15)<<data[i][j]<<" ";
            std::cout<<"\n";
        }
    }
    // Gaussian Elimination solver for Ax = b
    // Returns the solution vector x
    std::vector<T> solveGaussianElimination(std::vector<T> b)
    {
        if(rows!=cols)
            throw CircuitError("Matrix must be square for Gaussian Elimination.");
        if(rows!=b.size())
            throw CircuitError("Matrix and RHS vector dimensions mismatch for Gaussian Elimination.");
        size_t n=rows;
        std::vector<std::vector<T>> augmentedMatrix=data;
        for(size_t i=0; i<n; ++i)
            augmentedMatrix[i].push_back(b[i]); // Append b to form augmented matrix
        // Forward Elimination
        for(size_t k=0; k<n; ++k) 
        {
            // Find pivot row (partial pivoting)
            size_t pivotRow=k;
            // Use magnitude for pivoting for complex numbers or fabs for real numbers
            for(size_t i=k+1; i<n; ++i)
                if(get_magnitude(augmentedMatrix[i][k])>get_magnitude(augmentedMatrix[pivotRow][k]))
                    pivotRow=i;
            if(pivotRow!=k)
                std::swap(augmentedMatrix[k],augmentedMatrix[pivotRow]);
            // Check for singular matrix (using magnitude for complex numbers or fabs for real numbers)
            if(get_magnitude(augmentedMatrix[k][k])<1e-9) // Using a small epsilon for zero check
                throw CircuitError("Matrix is singular or ill-conditioned. Cannot solve with Gaussian Elimination.");
            // Eliminate
            for(size_t i=k+1; i<n; ++i) 
            {
                T factor=augmentedMatrix[i][k]/augmentedMatrix[k][k];
                for(size_t j=k; j<=n; ++j) // Iterate up to n (including b column)
                    augmentedMatrix[i][j]-=factor*augmentedMatrix[k][j];
            }
        }
        // Backward Substitution
        std::vector<T> x(n);
        for(int i=n-1; i>=0; --i) 
        {
            T sum=T(0.0);
            for(size_t j=i+1; j<n; ++j)
                sum+=augmentedMatrix[i][j]*x[j];
            x[i]=(augmentedMatrix[i][n]-sum)/augmentedMatrix[i][i];
        }
        return x;
    }
};
#endif 