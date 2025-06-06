// @HEADER
// *****************************************************************************
//        MueLu: A package for multigrid based preconditioning
//
// Copyright 2012 NTESS and the MueLu contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#ifndef MUELU_CONSTRAINT_DEF_HPP
#define MUELU_CONSTRAINT_DEF_HPP

#include <Teuchos_BLAS.hpp>
#include <Teuchos_LAPACK.hpp>
#include <Teuchos_SerialDenseVector.hpp>
#include <Teuchos_SerialDenseMatrix.hpp>
#include <Teuchos_SerialDenseHelpers.hpp>

#include <Xpetra_ImportFactory.hpp>
#include <Xpetra_Map.hpp>
#include <Xpetra_Matrix.hpp>
#include <Xpetra_MultiVectorFactory.hpp>
#include <Xpetra_MultiVector.hpp>
#include <Xpetra_CrsGraph.hpp>

#include "MueLu_Exceptions.hpp"

#include "MueLu_Constraint_decl.hpp"

namespace MueLu {

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
void Constraint<Scalar, LocalOrdinal, GlobalOrdinal, Node>::Setup(const RCP<MultiVector>& B, const RCP<MultiVector>& Bc, RCP<const CrsGraph> Ppattern) {
  B_  = B;
  Bc_ = Bc;

  const size_t NSDim = Bc->getNumVectors();

  Ppattern_ = Ppattern;

  size_t numRows = Ppattern_->getLocalNumRows();
  XXtInv_.resize(numRows);

  RCP<const Import> importer = Ppattern_->getImporter();

  X_ = MultiVectorFactory::Build(Ppattern_->getColMap(), NSDim);
  if (!importer.is_null())
    X_->doImport(*Bc, *importer, Xpetra::INSERT);
  else
    *X_ = *Bc;

  std::vector<const SC*> Xval(NSDim);
  for (size_t j = 0; j < NSDim; j++)
    Xval[j] = X_->getData(j).get();

  SC zero = Teuchos::ScalarTraits<SC>::zero();
  SC one  = Teuchos::ScalarTraits<SC>::one();

  Teuchos::BLAS<LO, SC> blas;
  Teuchos::LAPACK<LO, SC> lapack;
  LO lwork = 3 * NSDim;
  ArrayRCP<LO> IPIV(NSDim);
  ArrayRCP<SC> WORK(lwork);

  for (size_t i = 0; i < numRows; i++) {
    Teuchos::ArrayView<const LO> indices;
    Ppattern_->getLocalRowView(i, indices);

    size_t nnz = indices.size();

    XXtInv_[i]                                 = Teuchos::SerialDenseMatrix<LO, SC>(NSDim, NSDim, false /*zeroOut*/);
    Teuchos::SerialDenseMatrix<LO, SC>& XXtInv = XXtInv_[i];

    if (NSDim == 1) {
      SC d = zero;
      for (size_t j = 0; j < nnz; j++)
        d += Xval[0][indices[j]] * Xval[0][indices[j]];
      XXtInv(0, 0) = one / d;

    } else {
      Teuchos::SerialDenseMatrix<LO, SC> locX(NSDim, nnz, false /*zeroOut*/);
      for (size_t j = 0; j < nnz; j++)
        for (size_t k = 0; k < NSDim; k++)
          locX(k, j) = Xval[k][indices[j]];

      // XXtInv_ = (locX*locX^T)^{-1}
      blas.GEMM(Teuchos::NO_TRANS, Teuchos::CONJ_TRANS, NSDim, NSDim, nnz,
                one, locX.values(), locX.stride(),
                locX.values(), locX.stride(),
                zero, XXtInv.values(), XXtInv.stride());

      LO info;
      // Compute LU factorization using partial pivoting with row exchanges
      lapack.GETRF(NSDim, NSDim, XXtInv.values(), XXtInv.stride(), IPIV.get(), &info);
      // Use the computed factorization to compute the inverse
      lapack.GETRI(NSDim, XXtInv.values(), XXtInv.stride(), IPIV.get(), WORK.get(), lwork, &info);
    }
  }
}

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
typename Teuchos::ScalarTraits<Scalar>::magnitudeType
Constraint<Scalar, LocalOrdinal, GlobalOrdinal, Node>::ResidualNorm(const RCP<const Matrix> P) const {
  const auto one = Teuchos::ScalarTraits<Scalar>::one();

  auto residual = MultiVectorFactory::Build(B_->getMap(), B_->getNumVectors());
  P->apply(*Bc_, *residual, Teuchos::NO_TRANS);
  residual->update(one, *B_, -one);
  Teuchos::Array<MagnitudeType> norms(B_->getNumVectors());
  residual->norm2(norms);
  MagnitudeType residualNorm = Teuchos::ScalarTraits<MagnitudeType>::zero();
  for (size_t k = 0; k < B_->getNumVectors(); ++k) {
    residualNorm += norms[k] * norms[k];
  }
  return Teuchos::ScalarTraits<MagnitudeType>::squareroot(residualNorm);
}

//! \note We assume that the graph of Projected is the same as Ppattern_
template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
void Constraint<Scalar, LocalOrdinal, GlobalOrdinal, Node>::Apply(const Matrix& P, Matrix& Projected) const {
  // We check only row maps. Column may be different.
  TEUCHOS_TEST_FOR_EXCEPTION(!P.getRowMap()->isSameAs(*Projected.getRowMap()), Exceptions::Incompatible,
                             "Row maps are incompatible");
  const size_t NSDim   = X_->getNumVectors();
  const size_t numRows = P.getLocalNumRows();

  const Map& colMap  = *P.getColMap();
  const Map& PColMap = *Projected.getColMap();

  Projected.resumeFill();

  Teuchos::ArrayView<const LO> indices, pindices;
  Teuchos::ArrayView<const SC> values, pvalues;
  Teuchos::Array<SC> valuesAll(colMap.getLocalNumElements()), newValues;

  LO invalid = Teuchos::OrdinalTraits<LO>::invalid();
  LO oneLO   = Teuchos::OrdinalTraits<LO>::one();
  SC zero    = Teuchos::ScalarTraits<SC>::zero();
  SC one     = Teuchos::ScalarTraits<SC>::one();

  std::vector<const SC*> Xval(NSDim);
  for (size_t j = 0; j < NSDim; j++)
    Xval[j] = X_->getData(j).get();

  for (size_t i = 0; i < numRows; i++) {
    P.getLocalRowView(i, indices, values);
    Projected.getLocalRowView(i, pindices, pvalues);

    size_t nnz  = indices.size();   // number of nonzeros in the supplied matrix
    size_t pnnz = pindices.size();  // number of nonzeros in the constrained matrix

    newValues.resize(pnnz);

    // Step 1: fix stencil
    // Projected *must* already have the correct stencil

    // Step 2: copy correct stencil values
    // The algorithm is very similar to the one used in the calculation of
    // Frobenius dot product, see src/Transfers/Energy-Minimization/Solvers/MueLu_CGSolver_def.hpp

    // NOTE: using extra array allows us to skip the search among indices
    for (size_t j = 0; j < nnz; j++)
      valuesAll[indices[j]] = values[j];
    for (size_t j = 0; j < pnnz; j++) {
      LO ind = colMap.getLocalElement(PColMap.getGlobalElement(pindices[j]));  // FIXME: we could do that before the full loop just once
      if (ind != invalid)
        // index indices[j] is part of template, copy corresponding value
        newValues[j] = valuesAll[ind];
      else
        newValues[j] = zero;
    }
    for (size_t j = 0; j < nnz; j++)
      valuesAll[indices[j]] = zero;

    // Step 3: project to the space
    Teuchos::SerialDenseMatrix<LO, SC>& XXtInv = XXtInv_[i];

    Teuchos::SerialDenseMatrix<LO, SC> locX(NSDim, pnnz, false);
    for (size_t j = 0; j < pnnz; j++)
      for (size_t k = 0; k < NSDim; k++)
        locX(k, j) = Xval[k][pindices[j]];

    Teuchos::SerialDenseVector<LO, SC> val(pnnz, false), val1(NSDim, false), val2(NSDim, false);
    for (size_t j = 0; j < pnnz; j++)
      val[j] = newValues[j];

    Teuchos::BLAS<LO, SC> blas;
    // val1 = locX * val;
    blas.GEMV(Teuchos::NO_TRANS, NSDim, pnnz,
              one, locX.values(), locX.stride(),
              val.values(), oneLO,
              zero, val1.values(), oneLO);
    // val2 = XXtInv * val1
    blas.GEMV(Teuchos::NO_TRANS, NSDim, NSDim,
              one, XXtInv.values(), XXtInv.stride(),
              val1.values(), oneLO,
              zero, val2.values(), oneLO);
    // val = X^T * val2
    blas.GEMV(Teuchos::CONJ_TRANS, NSDim, pnnz,
              one, locX.values(), locX.stride(),
              val2.values(), oneLO,
              zero, val.values(), oneLO);

    for (size_t j = 0; j < pnnz; j++)
      newValues[j] -= val[j];

    Projected.replaceLocalValues(i, pindices, newValues);
  }

  Projected.fillComplete(Projected.getDomainMap(), Projected.getRangeMap());  // FIXME: maps needed?
}

}  // namespace MueLu

#endif  // ifndef MUELU_CONSTRAINT_DEF_HPP
