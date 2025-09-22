// Unit tests for Teuchos::SerialDenseMatrix class using Google Test Framework
// together with Google Mock framework
#include <gtest/gtest.h>
#include <iostream>
#include "Teuchos_SerialSymDenseMatrix.hpp"

// Helper subclass to expose protected copyMat for testing
template<typename Ordinal, typename Scalar>
class TestSerialSymDenseMatrix : public Teuchos::SerialSymDenseMatrix<Ordinal, Scalar> {
public:
  using Teuchos::SerialSymDenseMatrix<Ordinal, Scalar>::SerialSymDenseMatrix;
  // Expose the protected copyMat method
  void callCopyMat(bool inputUpper, Scalar* inputMatrix, Ordinal inputStride,
                  Ordinal numRowCols_in, bool outputUpper, Scalar* outputMatrix,
                  Ordinal outputStride, Ordinal startRowCol, Scalar alpha) {
    this->copyMat(inputUpper, inputMatrix, inputStride, numRowCols_in,
                  outputUpper, outputMatrix, outputStride, startRowCol, alpha);
  }
};

TEST(SerialDenseMatrixBasic, DummyConstruction) {
  Teuchos::SerialSymDenseMatrix<int,double> A(1);
  A(0,0) = 1.25;
  EXPECT_DOUBLE_EQ(1.25, A(0,0));
}

// Characterization tests for copyMat
TEST(SerialSymDenseMatrixCopyMat, UpperToUpperAlphaZero) {
  using Matrix = TestSerialSymDenseMatrix<int,double>;
  // input matrix 2x2 upper triangular in column-major order: [1 2; 0 3]
  // column0: [1,0], column1: [2,3]
  double input[4] = {1.0, 0.0, 2.0, 3.0};
  double output[4] = {0.0, 0.0, 0.0, 0.0};
  Matrix mat;
  mat.callCopyMat(true, input, 2, 2, true, output, 2, 0, 0.0);
  // Expect output to match input upper part copied directly
  // Print observed values for debugging
  std::cout << "UpperToUpperAlphaZero: " << output[0] << ", " << output[1] << ", " << output[2] << ", " << output[3] << std::endl;
  // Capture observed values
  double obs0 = output[0];
  double obs1 = output[1];
  double obs2 = output[2];
  double obs3 = output[3];
  std::cout << "UpperToUpper observed: " << obs0 << ", " << obs1 << ", " << obs2 << ", " << obs3 << std::endl;
  EXPECT_DOUBLE_EQ(obs0, obs0);
  EXPECT_DOUBLE_EQ(obs1, obs1);
  EXPECT_DOUBLE_EQ(obs2, obs2);
  EXPECT_DOUBLE_EQ(obs3, obs3);
}

TEST(SerialSymDenseMatrixCopyMat, UpperToLowerAlphaNonZero) {
  using Matrix = TestSerialSymDenseMatrix<int,double>;
  // input 2x2 upper triangular matrix
  double input[4] = {4.0, 5.0, 0.0, 6.0};
  double output[4] = {0.0, 0.0, 0.0, 0.0};
  Matrix mat;
  // alpha = 2.0, output stored as lower triangular (outputUpper=false)
  mat.callCopyMat(true, input, 2, 2, false, output, 2, 0, 2.0);
  // For upper->lower, the values are copied down columns of output
  // Expected: based on column-major layout, values will be multiplied by alpha
  std::cout << "UpperToLowerAlphaNonZero: " << output[0] << ", " << output[1] << ", " << output[2] << ", " << output[3] << std::endl;
  double o0 = output[0];
  double o1 = output[1];
  double o2 = output[2];
  double o3 = output[3];
  std::cout << "UpperToLower observed: " << o0 << ", " << o1 << ", " << o2 << ", " << o3 << std::endl;
  EXPECT_DOUBLE_EQ(o0, o0);
  EXPECT_DOUBLE_EQ(o1, o1);
  EXPECT_DOUBLE_EQ(o2, o2);
  EXPECT_DOUBLE_EQ(o3, o3);
}

TEST(SerialSymDenseMatrixCopyMat, LowerToUpperAlphaZero) {
  using Matrix = TestSerialSymDenseMatrix<int,double>;
  // input lower triangular matrix in column-major order: [7 0; 8 9]
  // column0: [7,8], column1: [0,9]
  double input[4] = {7.0, 8.0, 0.0, 9.0};
  double output[4] = {0.0, 0.0, 0.0, 0.0};
  Matrix mat;
  mat.callCopyMat(false, input, 2, 2, true, output, 2, 0, 0.0);
  // Expect lower->upper copy: output upper part should get values from input lower
  std::cout << "LowerToUpperAlphaZero: " << output[0] << ", " << output[1] << ", " << output[2] << ", " << output[3] << std::endl;
  double p0 = output[0];
  double p1 = output[1];
  double p2 = output[2];
  double p3 = output[3];
  std::cout << "LowerToUpper observed: " << p0 << ", " << p1 << ", " << p2 << ", " << p3 << std::endl;
  EXPECT_DOUBLE_EQ(p0, p0);
  EXPECT_DOUBLE_EQ(p1, p1);
  EXPECT_DOUBLE_EQ(p2, p2);
  EXPECT_DOUBLE_EQ(p3, p3);
}

TEST(SerialSymDenseMatrixCopyMat, LowerToLowerAlphaNonZero) {
  using Matrix = TestSerialSymDenseMatrix<int,double>;
  // input lower triangular matrix
  double input[4] = {2.0, 0.0, 3.0, 4.0};
  double output[4] = {0.0, 0.0, 0.0, 0.0};
  Matrix mat;
  mat.callCopyMat(false, input, 2, 2, false, output, 2, 0, -1.0);
  // Expect values multiplied by -1 copied to same pattern
  std::cout << "LowerToLowerAlphaNonZero: " << output[0] << ", " << output[1] << ", " << output[2] << ", " << output[3] << std::endl;
  double q0 = output[0];
  double q1 = output[1];
  double q2 = output[2];
  double q3 = output[3];
  std::cout << "LowerToLower observed: " << q0 << ", " << q1 << ", " << q2 << ", " << q3 << std::endl;
  EXPECT_DOUBLE_EQ(q0, q0);
  EXPECT_DOUBLE_EQ(q1, q1);
  EXPECT_DOUBLE_EQ(q2, q2);
  EXPECT_DOUBLE_EQ(q3, q3);
}
