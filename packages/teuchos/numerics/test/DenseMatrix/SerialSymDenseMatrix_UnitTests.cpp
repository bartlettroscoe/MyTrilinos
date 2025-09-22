// Unit tests for Teuchos::SerialDenseMatrix class using Google Test Framework
// together with Google Mock framework
#include <gtest/gtest.h>
#include "Teuchos_SerialSymDenseMatrix.hpp"

TEST(SerialDenseMatrixBasic, DummyConstruction) {
  Teuchos::SerialSymDenseMatrix<int,double> A(1);
  A(0,0) = 1.25;
  EXPECT_DOUBLE_EQ(1.25, A(0,0));
}

// Characterization tests for normFrobenius()
TEST(SerialSymDenseMatrixNormFrobenius, UpperTriangular) {
  // Create a 2x2 matrix and set it as upper triangular
  Teuchos::SerialSymDenseMatrix<int,double> A(2);
  A.setUpper();
  // Fill matrix entries (full matrix view)
  A(0,0) = 1.0;
  A(0,1) = 2.0;
  A(1,1) = 3.0;
  // Compute norm
  double norm = A.normFrobenius();
  // Dummy expected value; will be replaced after observing actual output
  double expected = 4.2426406871192848;
  EXPECT_DOUBLE_EQ(expected, norm);
}

TEST(SerialSymDenseMatrixNormFrobenius, LowerTriangular) {
  // Create a 2x2 matrix (default lower triangular)
  Teuchos::SerialSymDenseMatrix<int,double> A(2);
  A.setLower();
  // Fill matrix entries
  A(0,0) = 1.0;
  A(1,0) = 2.0; // lower off-diagonal element
  A(1,1) = 3.0;
  double norm = A.normFrobenius();
  double expected = 4.2426406871192848;
  EXPECT_DOUBLE_EQ(expected, norm);
}
