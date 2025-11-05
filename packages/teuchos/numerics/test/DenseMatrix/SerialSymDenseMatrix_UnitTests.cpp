// Unit tests for Teuchos::SerialDenseMatrix class using Google Test Framework
// together with Google Mock framework
#include <gtest/gtest.h>
#include "Teuchos_SerialSymDenseMatrix.hpp"

TEST(SerialDenseMatrixBasic, DummyConstruction) {
  Teuchos::SerialSymDenseMatrix<int,double> A(1);
  A(0,0) = 1.25;
  EXPECT_DOUBLE_EQ(1.25, A(0,0));
}

Teuchos::SerialSymDenseMatrix<int,double> createMatrix2x2Upper() {
  Teuchos::SerialSymDenseMatrix<int,double> M(2);
  // By default, active part is lower triangular; set upper
  M.setUpper();
  // Set values (upper triangular stored only)
  M(0,0) = 1.0;
  M(0,1) = 2.0; // off-diagonal
  M(1,1) = 3.0;
  return M;
}

Teuchos::SerialSymDenseMatrix<int,double> createMatrix2x2Lower() {
  Teuchos::SerialSymDenseMatrix<int,double> M(2);
  // default lower active
  M(0,0) = 1.0;
  M(1,0) = 2.0; // off-diagonal stored in lower part
  M(1,1) = 3.0;
  return M;
}

Teuchos::SerialSymDenseMatrix<int,double> createMatrix3x3Upper() {
  Teuchos::SerialSymDenseMatrix<int,double> M(3);
  M.setUpper();
  M(0,0) = 1.1; M(0,1) = 1.2; M(0,2) = 1.3;
                M(1,1) = 2.2; M(1,2) = 2.3;
                              M(2,2) = 3.3;
  return M;
}

Teuchos::SerialSymDenseMatrix<int,double> createMatrix3x3Lower() {
  Teuchos::SerialSymDenseMatrix<int,double> M(3);
  M(0,0) = 1.1;
  M(1,0) = 2.1; M(1,1) = 2.2;
  M(2,0) = 3.1; M(2,1) = 3.2; M(2,2) = 3.3;
  return M;
}

TEST(SerialSymDenseMatrixNorm, Upper2x2) {
  auto M = createMatrix2x2Upper();
  double result = M.normFrobenius();
  // Expected norm: sqrt(1^2 + 2^2 + 2^2 + 3^2) = sqrt(18)
  double expected = 4.242640687119285; // sqrt(18)
  EXPECT_DOUBLE_EQ(result, expected);
}

TEST(SerialSymDenseMatrixNorm, Lower2x2) {
  auto M = createMatrix2x2Lower();
  double result = M.normFrobenius();
  double expected = 4.242640687119285; // sqrt(18)
  EXPECT_DOUBLE_EQ(result, expected);
}

TEST(SerialSymDenseMatrixNorm, Upper3x3) {
  auto M = createMatrix3x3Upper();
  double result = M.normFrobenius();
  double expected = 5.8120564346881558;
  EXPECT_DOUBLE_EQ(result, expected);
}

TEST(SerialSymDenseMatrixNorm, Lower3x3) {
  auto M = createMatrix3x3Lower();
  double result = M.normFrobenius();
  double expected = 8.0907354424675137;
  EXPECT_DOUBLE_EQ(result, expected);
}
