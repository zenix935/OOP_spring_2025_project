#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
#include <iostream> // For operator<<
#include <iomanip>  // For setw, fixed, setprecision
#include <stdexcept> // For runtime_error
#include "Exceptions.h" // Include custom MatrixError
#include <cmath>    // For abs with doubles
#include <string>   // For string (for print title)
#include "Complex.h"   // <--- ADDED THIS LINE: Necessary for Complex's operator<< in generic print

// Forward declaration of Complex is no longer strictly needed here,
// but it doesn't hurt and can be useful for other contexts if Complex
// were only used by pointer/reference. However, since we need its full definition
// for value semantics and operator overloads in the template, include is essential.
// class Complex; // Can remove this if desired, as Complex.h will define it.

using namespace std;

template <typename T>
class Matrix {
private:
    vector<vector<T>> data;
    size_t rows;
    size_t cols;

public:
    // Constructor
    Matrix(size_t r=0,size_t c=0) : rows(r),cols(c) {
        data.resize(rows,vector<T>(cols,T{})); // Initialize with default value of T
    }

    // Resize matrix
    void resize(size_t r,size_t c) {
        rows=r;
        cols=c;
        data.assign(rows,vector<T>(cols,T{})); // Reinitialize with default value
    }

    // Getters for dimensions
    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }

    // Access element (read/write)
    T& operator()(size_t r,size_t c) {
        if(r>=rows||c>=cols) {
            throw out_of_range("Matrix index out of bounds.");
        }
        return data[r][c];
    }

    // Access element (read-only)
    const T& operator()(size_t r,size_t c) const {
        if(r>=rows||c>=cols) {
            throw out_of_range("Matrix index out of bounds.");
        }
        return data[r][c];
    }

    // Add a value to an existing element (useful for MNA stamping)
    void add(size_t r,size_t c,const T& value) {
        if(r>=rows||c>=cols) {
            throw out_of_range("Matrix add index out of bounds.");
        }
        data[r][c]+=value;
    }

    // Print matrix (for debugging) - MODIFIED to accept a title
    void print(const string& title="") const {
        if(!title.empty()) {
            cout<<"\n--- "<<title<<" ---\n";
        }
        cout<<"Matrix ("<<rows<<"x"<<cols<<"):\n";
        for(size_t i=0; i<rows; ++i) {
            for(size_t j=0; j<cols; ++j) {
                // This line now correctly resolves operator<< for Complex as well
                // because Complex.h is included.
                cout<<setw(12)<<fixed<<setprecision(4)<<data[i][j]<<" ";
            }
            cout<<"\n";
        }
    }

    // Gaussian Elimination solver
    vector<T> solveGaussianElimination(vector<T> b) const {
        if(rows==0||cols==0) {
            throw MatrixError("Cannot solve empty matrix.");
        }
        if(rows!=cols) {
            throw MatrixError("Matrix must be square for Gaussian Elimination.");
        }
        if(b.size()!=rows) {
            throw MatrixError("RHS vector size must match matrix dimensions.");
        }

        Matrix<T> A_copy=*this; // Create a mutable copy of the matrix
        vector<T> x(rows); // Solution vector

        // Forward elimination
        for(size_t k=0; k<rows; ++k) {
            // Find pivot (largest absolute value in current column below or at k)
            size_t pivot_row=k;
            for(size_t i=k+1; i<rows; ++i) {
                // Use abs() which will correctly dispatch to std::abs for double
                // and our custom abs for Complex.
                if(abs(A_copy(i,k))>abs(A_copy(pivot_row,k))) {
                    pivot_row=i;
                }
            }

            if(pivot_row!=k) {
                // Swap rows in A_copy
                swap(A_copy.data[k],A_copy.data[pivot_row]);
                // Swap rows in b
                swap(b[k],b[pivot_row]);
            }

            // Check for singular matrix (pivot element is zero or very close to zero)
            // Use abs() for the check
            if(abs(A_copy(k,k))<1e-12) {
                throw MatrixError("Matrix is singular or ill-conditioned, cannot solve.");
            }

            // Eliminate column k
            for(size_t i=k+1; i<rows; ++i) {
                T factor=A_copy(i,k)/A_copy(k,k);
                for(size_t j=k; j<cols; ++j) { // Iterate from k, not k+1 because A_copy(i,k) will become 0
                    A_copy(i,j)-=factor*A_copy(k,j);
                }
                b[i]-=factor*b[k];
            }
        }

        // Back substitution
        for(long i=rows-1; i>=0; --i) {
            T sum_terms=T{}; // Initialize with default value (0 for double, Complex(0,0) for Complex)
            for(size_t j=i+1; j<cols; ++j) {
                sum_terms+=A_copy(i,j)*x[j];
            }
            x[i]=(b[i]-sum_terms)/A_copy(i,i);
        }

        return x;
    }
};

// Explicit specialization of Matrix for Complex to ensure correct printing
// This specialization is still useful if you want different formatting for Complex numbers,
// but the generic `print` method will now also work.
template<>
inline void Matrix<Complex>::print(const string& title) const {
    if(!title.empty()) {
        cout<<"\n--- "<<title<<" ---\n";
    }
    cout<<"Matrix ("<<rows<<"x"<<cols<<"):\n";
    for(size_t i=0; i<rows; ++i) {
        for(size_t j=0; j<cols; ++j) {
            // This relies on the Complex::operator<< which handles its own formatting
            cout<<setw(12)<<data[i][j]<<" ";
        }
        cout<<"\n";
    }
}

#endif // MATRIX_H