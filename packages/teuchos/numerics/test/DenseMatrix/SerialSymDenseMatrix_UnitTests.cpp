// Unit tests for Teuchos::SerialDenseMatrix class using Google Test Framework
// together with Google Mock framework
#include <gtest/gtest.h>
#include "Teuchos_SerialSymDenseMatrix.hpp"

TEST(SerialDenseMatrixBasic, DummyConstruction) {
  Teuchos::SerialSymDenseMatrix<int,double> A(1);
  A(0,0) = 1.25;
  EXPECT_DOUBLE_EQ(1.25, A(0,0));
}

// Characterization tests for normFrobenius (upper triangular storage)
TEST(SerialSymDenseMatrixNormFrobenius, UpperTriangular) {
  // Create a 2x2 symmetric matrix
  Teuchos::SerialSymDenseMatrix<int,double> A(2);
  // Use upper storage
  A.setUpper();
  // Fill matrix entries (symmetric)
  A(0,0) = 1.0;
  A(0,1) = 2.0; // off-diagonal
  A(1,1) = 3.0;
  double result = A.normFrobenius();
  // Dummy expected value; will be replaced after observing actual output
  EXPECT_DOUBLE_EQ(result, 4.2426406871192848);
}

// Characterization tests for normFrobenius (lower triangular storage)
TEST(SerialSymDenseMatrixNormFrobenius, LowerTriangular) {
  // Create a 2x2 symmetric matrix with default (lower) storage
  Teuchos::SerialSymDenseMatrix<int,double> A(2);
  // Ensure lower storage (default)
  // Fill matrix entries (symmetric)
  A(0,0) = 1.0;
  A(0,1) = 2.0; // off-diagonal
  A(1,1) = 3.0;
  double result = A.normFrobenius();
  // Dummy expected value; will be replaced after observing actual output
  EXPECT_DOUBLE_EQ(result, 4.2426406871192848);
}
