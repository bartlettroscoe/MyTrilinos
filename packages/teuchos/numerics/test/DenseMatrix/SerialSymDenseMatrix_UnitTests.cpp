// Unit tests for Teuchos::SerialDenseMatrix class using Google Test Framework
// together with Google Mock framework
#include <gtest/gtest.h>
#include "Teuchos_SerialSymDenseMatrix.hpp"

TEST(SerialDenseMatrixBasic, DummyConstruction) {
  Teuchos::SerialSymDenseMatrix<int,double> A(1);
  A(0,0) = 1.25;
  EXPECT_DOUBLE_EQ(1.25, A(0,0));
}

// Characterization tests for normFrobenius
TEST(SerialSymDenseMatrixNorm, ZeroMatrix) {
  Teuchos::SerialSymDenseMatrix<int,double> A(3, true); // upper flag not relevant for zero
  // No values set, remains zero
  auto result = A.normFrobenius();
  EXPECT_DOUBLE_EQ(0.0, result); // observed
}

TEST(SerialSymDenseMatrixNorm, OneByOne) {
  Teuchos::SerialSymDenseMatrix<int,double> A(1);
  A(0,0) = 3.0;
  auto result = A.normFrobenius();
  EXPECT_DOUBLE_EQ(3.0, result); // observed
}

TEST(SerialSymDenseMatrixNorm, TwoByTwoLower) {
  Teuchos::SerialSymDenseMatrix<int,double> A(2);
  A(0,0) = 1.0;
  A(1,0) = 2.0; // lower off-diagonal
  A(1,1) = 3.0;
  auto result = A.normFrobenius();
  EXPECT_DOUBLE_EQ(4.2426406871192848, result); // observed
}

TEST(SerialSymDenseMatrixNorm, TwoByTwoUpper) {
  // Create upper matrix using constructor with upper flag
  Teuchos::SerialSymDenseMatrix<int,double> A(2, true);
  A(0,0) = 1.0;
  A(0,1) = 4.0; // upper off-diagonal
  A(1,1) = 5.0;
  auto result = A.normFrobenius();
  EXPECT_DOUBLE_EQ(7.6157731058639087, result); // observed
}
