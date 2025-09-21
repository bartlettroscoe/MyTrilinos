// Unit tests for Teuchos::SerialDenseMatrix class using Google Test Framework
// together with Google Mock framework
#include <gtest/gtest.h>
#include "Teuchos_SerialSymDenseMatrix.hpp"

TEST(SerialDenseMatrixBasic, DummyConstruction) {
  Teuchos::SerialSymDenseMatrix<int,double> A(1);
  A(0,0) = 1.25;
  EXPECT_DOUBLE_EQ(1.25, A(0,0));
}

// Test the normFrobenius() method for various matrix configurations.
// These tests exercise the computation for upper and lower triangular
// representations, as well as edge cases such as 0x0 and 1x1 matrices.

// Helper to compute expected Frobenius norm for a given raw array.
static double expectedNorm(const double *values, int n, bool upper) {
  double sum = 0.0;
  if (upper) {
    for (int j = 0; j < n; ++j) {
      for (int i = 0; i < j; ++i) {
        double v = values[i + j * n];
        sum += 2.0 * v * v;
      }
      double v = values[j + j * n];
      sum += v * v;
    }
  } else {
    for (int j = 0; j < n; ++j) {
      double v = values[j + j * n];
      sum += v * v;
      for (int i = j + 1; i < n; ++i) {
        double w = values[i + j * n];
        sum += 2.0 * w * w;
      }
    }
  }
  return std::sqrt(sum);
}

TEST(SerialSymDenseMatrixNorm, ZeroSize) {
  Teuchos::SerialSymDenseMatrix<int,double> A(0);
  EXPECT_DOUBLE_EQ(0.0, A.normFrobenius());
}

TEST(SerialSymDenseMatrixNorm, OneByOne) {
  double vals[1] = {3.0};
  Teuchos::SerialSymDenseMatrix<int,double> A(Teuchos::Copy, true, vals, 1, 1);
  EXPECT_DOUBLE_EQ(3.0, A.normFrobenius());
}

TEST(SerialSymDenseMatrixNorm, TwoByTwoUpper) {
  double vals[4] = {1.0, 2.0, 2.0, 3.0}; // column-major representation
  Teuchos::SerialSymDenseMatrix<int,double> A(Teuchos::Copy, true, vals, 2, 2);
  double expected = expectedNorm(vals, 2, true);
  EXPECT_DOUBLE_EQ(expected, A.normFrobenius());
}

TEST(SerialSymDenseMatrixNorm, TwoByTwoLower) {
  double vals[4] = {1.0, 2.0, 2.0, 3.0};
  Teuchos::SerialSymDenseMatrix<int,double> A(Teuchos::Copy, false, vals, 2, 2);
  double expected = expectedNorm(vals, 2, false);
  EXPECT_DOUBLE_EQ(expected, A.normFrobenius());
}

TEST(SerialSymDenseMatrixNorm, ThreeByThreeUpper) {
  double vals[9] = {1,2,3,4,5,6,7,8,9};
  Teuchos::SerialSymDenseMatrix<int,double> A(Teuchos::Copy, true, vals, 3, 3);
  double expected = expectedNorm(vals, 3, true);
  EXPECT_DOUBLE_EQ(expected, A.normFrobenius());
}

TEST(SerialSymDenseMatrixNorm, ThreeByThreeLower) {
  double vals[9] = {1,2,3,4,5,6,7,8,9};
  Teuchos::SerialSymDenseMatrix<int,double> A(Teuchos::Copy, false, vals, 3, 3);
  double expected = expectedNorm(vals, 3, false);
  EXPECT_DOUBLE_EQ(expected, A.normFrobenius());
}
