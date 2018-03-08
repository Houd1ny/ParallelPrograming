#include <iostream>
#include <iomanip>

using namespace std;


class Matrix
{
public:
    Matrix(int n)
    {
        this->n = n;
        data = new int* [n];
        for (int i = 0; i < n; ++i)
        {
            data[i] = new int [n];
        }
    }

    ~Matrix()
    {
        for (int i = 0; i < n; ++i)
        {
            delete [] data[i];
        }
        delete [] data;
    }

    Matrix(Matrix const& m)
        :Matrix(m.n)
    {
        *this = m;
    }

    Matrix(Matrix const& m, int rowStart, int rowEnd, int colStart, int colEnd)
        :Matrix(rowEnd - rowStart)
    {
        for (int i = rowStart; i < rowEnd; ++i)
        {
            for (int j = colStart; j < colEnd; ++j)
            {
                (*this)(i-rowStart, j-colStart) = m(i, j);
            }
        }
    }

    void copyPart(Matrix const& m, int rowStart, int rowEnd, int colStart, int colEnd)
    {
        for (int i = rowStart; i < rowEnd; ++i)
        {
            for (int j = colStart; j < colEnd; ++j)
            {
                (*this)(rowStart, colStart) = m(i-rowStart, j-colStart);
            }
        }
    }

    Matrix& operator = (Matrix const& m)
    {
        if (this != &m)
        {
            for (int i = 0; i < n; ++i)
            {
                for (int j = 0; j < n; ++j)
                {
                    (*this)(i, j) = m(i, j);
                }
            }
        }
        return *this;
    }

    Matrix(Matrix && m)
    {
        this->data = m.data;
    }

    int& operator()(int i, int j)
    {
        return data[i][j];
    }

    int operator()(int i, int j) const
    {
        return data[i][j];
    }

    Matrix& operator=(Matrix && m)
    {
        if (this != &m)
        {
            this->data = m.data;
        }
        return *this;
    }

    Matrix operator*(const Matrix& m)
    {
        Matrix result(m.n);
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                result(i, j) = 0;
                for (int k = 0; k < n; k++)
                {
                    result(i,j) += (*this)(i, k) * m(k, j);
                }
            }
        }
        return result;
    }

    Matrix& operator+=(Matrix& m)
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                (*this)(i, j) += m(i, j);
            }
        }
        return *this;
    }

    Matrix& operator-=(Matrix& m)
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n; j++)
            {
                (*this)(i, j) -= m(i, j);
            }
        }
        return *this;
    }

    Matrix operator+(Matrix&& m)
    {
        Matrix result(m);
        return result += *this;
    }

    Matrix operator+(Matrix& m)
    {
        Matrix result(m);
        return result += *this;
    }

    Matrix operator-(Matrix&& m)
    {
        Matrix result(m);
        return -1*(result -= *this);
    }

    Matrix operator-(Matrix& m)
    {
        Matrix result(m);
        return -1*(result -= *this);
    }

    friend Matrix operator*(int scalar, const Matrix& matrix)
    {
        Matrix result(matrix);
        for (int i = 0; i < result.n; i++)
        {
            for (int j = 0; j < result.n; j++)
            {
                result(i, j) *= scalar;
            }
        }
        return result;
    }

    friend Matrix operator*(int scalar, const Matrix&& matrix)
    {
        Matrix result(matrix);
        for (int i = 0; i < result.n; i++)
        {
            for (int j = 0; j < result.n; j++)
            {
                result(i, j) *= scalar;
            }
        }
        return result;
    }

    friend std::ostream& operator<< (std::ostream& stream, const Matrix& matrix)
    {
        for (int i = 0; i < matrix.n; i++)
        {
            for (int j = 0; j < matrix.n; j++)
            {
                stream << setw(3) << matrix(i, j);
            }
            stream << endl;
        }
        return stream;
    }
public:
    int n;
    int scalar;
private:
    int** data;
};


/*
     X           Y                X*Y
 +-------+   +-------+     +-------+-------+
 | A | B |   | E | F |     | AE+BG | AF+BH |
 +---+---+ * +---+---+  =  +-------+-------+
 | C | D |   | G | H |     | CE+DG | CF+DH |
 +---+---+   +---+---+     +---------------+
 Seven products:
 P1 = A(F-H)
 P2 = (A+B)H
 P3 = (C+D)E
 P4 = D(G-E)
 P5 = (A+D)(E+H)
 P6 = (B-D)(G+H)
 P7 = (A-C)(E+F)

         +-------------+-------------+
         | P5+P4-P2+P6 |    P1+P2    |
 X * Y = +-------------+-------------+
         |    P3+P4    | P1+P5-P3+P7 |
         +-------------+-------------+
*/

Matrix strassen(const Matrix& left, const Matrix& right)
{
    int n = left.n;
    Matrix A(left, 0, n/2, 0, n/2);
    Matrix B(left, 0, n/2, n/2, n);
    Matrix C(left, n/2, n, 0, n/2);
    Matrix D(left, n/2, n, n/2, n);

    Matrix E(right, 0, n/2, 0, n/2);
    Matrix F(right, 0, n/2, n/2, n);
    Matrix G(right, n/2, n, 0, n/2);
    Matrix H(right, n/2, n, n/2, n);

    Matrix P1 = A*(F-H);
    Matrix P2 = (A+B)*H;
    Matrix P3 = (C+D)*E;
    Matrix P4 = D*(G-E);
    Matrix P5 = (A+D)*(E+H);
    Matrix P6 = (B-D)*(G+H);
    Matrix P7 = (A-C)*(E+F);

    Matrix result(left.n);
    result.copyPart(P5+P4-P2+P6, 0, n/2, 0, n/2);
    result.copyPart(P1+P2, 0, n/2, n/2, n);
    result.copyPart(P3+P4, n/2, n, 0, n/2);
    result.copyPart(P1+P5-P3-P7, n/2, n, n/2, n);

    return result;
}

int main()
{
    Matrix a(2);
    a(0, 0) = 1;
    a(0, 1) = 2;
    a(1, 0) = 3;
    a(1, 1) = 4;
    Matrix b(2);
    b(0, 0) = 2;
    b(0, 1) = 0;
    b(1, 0) = 1;
    b(1, 1) = 2;
    cout << strassen(a, b) << endl;

    return 0;
}

