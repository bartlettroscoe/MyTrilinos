// Unit tests for Teuchos::SerialDenseMatrix class using Google Test Framework
// together with Google Mock framework
#include <gtest/gtest.h>
#include <iostream>
#include "Teuchos_SerialSymDenseMatrix.hpp"

TEST(SerialDenseMatrixBasic, DummyConstruction) {
  Teuchos::SerialSymDenseMatrix<int,double> A(1);
  A(0,0) = 1.25;
  EXPECT_DOUBLE_EQ(1.25, A(0,0));
}

// Characterization tests for normFrobenius()
// These tests will initially use dummy expected values (0.0) and print the actual
// computed norm so that the expected values can be replaced with the observed ones.

TEST(SerialSymDenseMatrixNormFrobenius, LowerTriangular2x2) {
  Teuchos::SerialSymDenseMatrix<int, double> A(2); // lower active by default, zeroed
  // Set lower triangular elements
  A(0,0) = 1.0;
  A(1,0) = 2.0; // symmetric element (0,1) is implicitly 2.0
  A(1,1) = 3.0;
  double norm = A.normFrobenius();
  // Print actual value for observation
  std::cout << "LowerTriangular2x2 norm=" << norm << std::endl;
  // Dummy expected value; will be replaced after observation
  EXPECT_DOUBLE_EQ(norm, 4.2426406871192848);
}

TEST(SerialSymDenseMatrixNormFrobenius, UpperTriangular2x2) {
  Teuchos::SerialSymDenseMatrix<int, double> A(2);
  A.setUpper(); // activate upper triangle
  // Set upper triangular elements
  A(0,0) = 1.0;
  A(0,1) = 2.0; // symmetric element (1,0) will be 2.0
  A(1,1) = 3.0;
  double norm = A.normFrobenius();
  std::cout << "UpperTriangular2x2 norm=" << norm << std::endl;
  EXPECT_DOUBLE_EQ(norm, 4.2426406871192848);
}

TEST(SerialSymDenseMatrixNormFrobenius, SingleElement) {
  Teuchos::SerialSymDenseMatrix<int, double> A(1);
  A(0,0) = 5.5;
  double norm = A.normFrobenius();
  std::cout << "SingleElement norm=" << norm << std::endl;
  EXPECT_DOUBLE_EQ(norm, 5.5);
}

TEST(SerialSymDenseMatrixNormFrobenius, ZeroMatrix) {
  Teuchos::SerialSymDenseMatrix<int, double> A(3); // zeroed by default
  double norm = A.normFrobenius();
  std::cout << "ZeroMatrix norm=" << norm << std::endl;
  EXPECT_DOUBLE_EQ(norm, 0.0);
}
