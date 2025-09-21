// Unit tests for Teuchos::SerialDenseMatrix class using Google Test Framework
// together with Google Mock framework
#include <cmath>
#include <gtest/gtest.h>
#include "Teuchos_SerialSymDenseMatrix.hpp"

TEST(SerialDenseMatrixBasic, DummyConstruction) {
  Teuchos::SerialSymDenseMatrix<int,double> A(1);
  A(0,0) = 1.25;
  EXPECT_DOUBLE_EQ(1.25, A(0,0));
}

TEST(SerialSymDenseMatrixNormFrobenius, LowerStorageAggregatesAllEntries) {
  Teuchos::SerialSymDenseMatrix<int,double> A(3);
  A(0,0) = 1.0;
  A(1,1) = 2.0;
  A(2,2) = 3.0;
  A(1,0) = 4.0;
  A(2,0) = 5.0;
  A(2,1) = 6.0;

  const double result = A.normFrobenius();
  const double expected = std::sqrt(168.0);
  EXPECT_DOUBLE_EQ(expected, result);
}

TEST(SerialSymDenseMatrixNormFrobenius, UpperStorageUsesActiveTriangle) {
  Teuchos::SerialSymDenseMatrix<int,double> A(3);
  A.setUpper();
  A(0,0) = 1.0;
  A(1,1) = -2.0;
  A(2,2) = 3.0;
  A(0,1) = 0.25;
  A(0,2) = -1.5;
  A(1,2) = 2.5;

  const double result = A.normFrobenius();
  const double expected = std::sqrt(31.125);
  EXPECT_DOUBLE_EQ(expected, result);
}
