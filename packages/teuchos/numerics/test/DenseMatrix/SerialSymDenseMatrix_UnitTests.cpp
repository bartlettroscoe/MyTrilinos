// Unit tests for Teuchos::SerialDenseMatrix class using Google Test Framework
// together with Google Mock framework
#include <gtest/gtest.h>
#include "Teuchos_SerialSymDenseMatrix.hpp"

TEST(SerialDenseMatrixBasic, DummyConstruction) {
  Teuchos::SerialSymDenseMatrix<int,double> A(1);
  A(0,0) = 1.25;
  EXPECT_DOUBLE_EQ(1.25, A(0,0));
}

TEST(SerialSymDenseMatrixNormFrobenius, UsesLowerTriangleByDefault) {
  Teuchos::SerialSymDenseMatrix<int,double> matrix(3, true);
  matrix(0,0) = 1.0;
  matrix(1,0) = -2.0;
  matrix(1,1) = 3.0;
  matrix(2,0) = 4.0;
  matrix(2,1) = -5.0;
  matrix(2,2) = 6.0;

  const double result = matrix.normFrobenius();
  EXPECT_DOUBLE_EQ(11.661903789690601, result);
}

TEST(SerialSymDenseMatrixNormFrobenius, HonorsUpperTriangleWhenSelected) {
  Teuchos::SerialSymDenseMatrix<int,double> matrix(3, true);
  matrix.setUpper();
  matrix(0,0) = 1.5;
  matrix(0,1) = -2.5;
  matrix(0,2) = 3.5;
  matrix(1,1) = -4.5;
  matrix(1,2) = 5.5;
  matrix(2,2) = -6.5;

  const double result = matrix.normFrobenius();
  EXPECT_DOUBLE_EQ(12.737739202856996, result);
}
