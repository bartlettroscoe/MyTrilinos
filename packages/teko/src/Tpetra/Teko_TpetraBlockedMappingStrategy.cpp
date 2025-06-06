// @HEADER
// *****************************************************************************
//      Teko: A package for block and physics based preconditioning
//
// Copyright 2010 NTESS and the Teko contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#include "Teko_TpetraBlockedMappingStrategy.hpp"
#include "Teko_TpetraHelpers.hpp"

#include "Thyra_TpetraThyraWrappers.hpp"
#include "Thyra_TpetraLinearOp.hpp"
#include "Thyra_DefaultProductMultiVector.hpp"
#include "Thyra_DefaultProductVectorSpace.hpp"
#include "Thyra_DefaultSpmdMultiVector.hpp"
#include "Thyra_DefaultBlockedLinearOp.hpp"

using Teuchos::RCP;
using Teuchos::rcp;
using Teuchos::rcp_dynamic_cast;

namespace Teko {
namespace TpetraHelpers {

// Creates a strided mapping strategy. This class is useful
// for breaking up nodally ordered matrices (i.e. the unknowns
// in a FEM problem are ordered [u0,v0,p0,u1,v1,p1,...]). Current
// implimentation only supports a fixed number of variables
//
//    arguments:
//       vars - Number of different variables
//       map  - original Tpetra::Map<LO,GO,NT>  to be broken up
//       comm - Teuchos::RCP<Teuchos::Comm<int> > object related to the map
//
TpetraBlockedMappingStrategy::TpetraBlockedMappingStrategy(
    const std::vector<std::vector<GO>>& vars,
    const Teuchos::RCP<const Tpetra::Map<LO, GO, NT>>& map, const Teuchos::Comm<int>& comm) {
  rangeMap_  = map;
  domainMap_ = map;
  buildBlockTransferData(vars, rangeMap_, comm);
}

// Virtual function defined in MappingStrategy.  This copies
// an Tpetra::MultiVector<ST,LO,GO,NT> into a Thyra::MultiVectorBase with
// blocking handled by the strides defined in the constructor.
//
//   arguments:
//      X       - source Tpetra::MultiVector<ST,LO,GO,NT>
//      thyra_X - destination Thyra::MultiVectorBase
//
void TpetraBlockedMappingStrategy::copyTpetraIntoThyra(
    const Tpetra::MultiVector<ST, LO, GO, NT>& X,
    const Teuchos::Ptr<Thyra::MultiVectorBase<ST>>& thyra_X) const {
  int count = X.getNumVectors();

  std::vector<RCP<Tpetra::MultiVector<ST, LO, GO, NT>>> subX;

  // allocate vectors to copy into
  Blocking::buildSubVectors(blockMaps_, subX, count);

  // copy source vector to X vector
  Blocking::one2many(subX, X, blockImport_);

  // convert subX to an array of multi vectors
  Teuchos::Array<RCP<Thyra::MultiVectorBase<ST>>> thyra_subX;
  Teuchos::Ptr<Thyra::ProductMultiVectorBase<ST>> prod_X =
      Teuchos::ptr_dynamic_cast<Thyra::ProductMultiVectorBase<ST>>(thyra_X);
  for (unsigned int i = 0; i < blockMaps_.size(); i++) {
    RCP<Thyra::TpetraMultiVector<ST, LO, GO, NT>> vec =
        rcp_dynamic_cast<Thyra::TpetraMultiVector<ST, LO, GO, NT>>(
            prod_X->getNonconstMultiVectorBlock(i), true);

    fillDefaultSpmdMultiVector(vec, subX[i]);
  }
}

// Virtual function defined in MappingStrategy.  This copies
// an Tpetra::MultiVector<ST,LO,GO,NT> into a Thyra::MultiVectorBase with
// blocking handled by the strides defined in the constructor.
//
//   arguments:
//      thyra_Y - source Thyra::MultiVectorBase
//      Y       - destination Tpetra::MultiVector<ST,LO,GO,NT>
//
void TpetraBlockedMappingStrategy::copyThyraIntoTpetra(
    const RCP<const Thyra::MultiVectorBase<ST>>& thyra_Y,
    Tpetra::MultiVector<ST, LO, GO, NT>& Y) const {
  std::vector<RCP<const Tpetra::MultiVector<ST, LO, GO, NT>>> subY;
  RCP<const Thyra::DefaultProductMultiVector<ST>> prod_Y =
      rcp_dynamic_cast<const Thyra::DefaultProductMultiVector<ST>>(thyra_Y);

  // convert thyra product vector to subY
  for (unsigned int i = 0; i < blockMaps_.size(); i++) {
    RCP<const Thyra::TpetraMultiVector<ST, LO, GO, NT>> tmv =
        rcp_dynamic_cast<const Thyra::TpetraMultiVector<ST, LO, GO, NT>>(
            prod_Y->getMultiVectorBlock(i), true);
    subY.push_back(tmv->getConstTpetraMultiVector());
  }

  // endow the subVectors with required information about the maps
  // Blocking::associateSubVectors(blockMaps_,subY);

  // copy solution vectors to Y vector
  Blocking::many2one(Y, subY, blockExport_);
}

// this is the core routine that builds the maps
// and importers/exporters neccessary for all the
// transfers. Currently it simply calls out to the
// interlaced tpetra functions. (Comment: this
// routine should probably be private or protected
// ... it is basically the meat of the constructor)
//
//    arguments:
//       vars - Vector describing the blocking of variables
//       baseMap - basic map to use in the transfers
//       comm    - Teuchos::RCP<Teuchos::Comm<int> > object
//
void TpetraBlockedMappingStrategy::buildBlockTransferData(
    const std::vector<std::vector<GO>>& vars,
    const Teuchos::RCP<const Tpetra::Map<LO, GO, NT>>& baseMap, const Teuchos::Comm<int>& comm) {
  // build block for each vector
  for (std::size_t i = 0; i < vars.size(); i++) {
    // build maps and exporters/importers
    Blocking::MapPair mapPair = Blocking::buildSubMap(vars[i], comm);
    Blocking::ImExPair iePair = Blocking::buildExportImport(*baseMap, mapPair);

    blockMaps_.push_back(mapPair);
    blockImport_.push_back(iePair.first);
    blockExport_.push_back(iePair.second);
  }
}

// Builds a blocked Thyra operator that uses the strided
// mapping strategy to define sub blocks.
//
//    arguments:
//       mat - Tpetra::CrsMatrix<ST,LO,GO,NT>  with FillComplete called, this
//             matrix is assumed to be square, with the same
//             range and domain maps
//    returns: Blocked Thyra linear operator with sub blocks
//             defined by this mapping strategy
//
const Teuchos::RCP<Thyra::BlockedLinearOpBase<ST>>
TpetraBlockedMappingStrategy::buildBlockedThyraOp(
    const RCP<const Tpetra::CrsMatrix<ST, LO, GO, NT>>& crsContent,
    const std::string& label) const {
  int dim = blockMaps_.size();

  if (dim == 1) {
    RCP<Thyra::DefaultBlockedLinearOp<ST>> A = Thyra::defaultBlockedLinearOp<ST>();

    A->beginBlockFill(dim, dim);
    A->setBlock(
        0, 0,
        Thyra::constTpetraLinearOp<ST, LO, GO, NT>(
            Thyra::tpetraVectorSpace<ST, LO, GO, NT>(crsContent->getRangeMap()),
            Thyra::tpetraVectorSpace<ST, LO, GO, NT>(crsContent->getDomainMap()), crsContent));
    A->endBlockFill();

    return A;
  }

  plocal2ContigGIDs.resize(dim);
  for (int j = 0; j < dim; j++) {
    plocal2ContigGIDs[j] = Blocking::getSubBlockColumnGIDs(*crsContent, blockMaps_[j]);
  }

  RCP<Thyra::DefaultBlockedLinearOp<ST>> A = Thyra::defaultBlockedLinearOp<ST>();

  A->beginBlockFill(dim, dim);
  for (int i = 0; i < dim; i++) {
    for (int j = 0; j < dim; j++) {
      // label block correctly
      std::stringstream ss;
      ss << label << "_" << i << "," << j;

      // build the blocks and place it the right location
      RCP<Tpetra::CrsMatrix<ST, LO, GO, NT>> blk =
          Blocking::buildSubBlock(i, j, crsContent, blockMaps_, plocal2ContigGIDs[j]);
      A->setNonconstBlock(i, j,
                          Thyra::tpetraLinearOp<ST, LO, GO, NT>(
                              Thyra::tpetraVectorSpace<ST, LO, GO, NT>(blk->getRangeMap()),
                              Thyra::tpetraVectorSpace<ST, LO, GO, NT>(blk->getDomainMap()), blk));
    }
  }  // end for i
  A->endBlockFill();

  return A;
}

// Rebuilds a blocked Thyra operator that uses the strided
// mapping strategy to define sub blocks.
//
//    arguments:
//       crsContent - Tpetra::CrsMatrix<ST,LO,GO,NT>  with FillComplete called, this
//                    matrix is assumed to be square, with the same
//                    range and domain maps
//       A - Destination block linear op composed of blocks of
//           Tpetra::CrsMatrix<ST,LO,GO,NT>  at all relevant locations
//
void TpetraBlockedMappingStrategy::rebuildBlockedThyraOp(
    const RCP<const Tpetra::CrsMatrix<ST, LO, GO, NT>>& crsContent,
    const RCP<Thyra::BlockedLinearOpBase<ST>>& A) const {
  int dim = blockMaps_.size();

  if (dim == 1) return;

  for (int i = 0; i < dim; i++) {
    for (int j = 0; j < dim; j++) {
      // get Tpetra version of desired block
      RCP<Thyra::LinearOpBase<ST>> Aij = A->getNonconstBlock(i, j);
      RCP<Thyra::TpetraLinearOp<ST, LO, GO, NT>> tAij =
          rcp_dynamic_cast<Thyra::TpetraLinearOp<ST, LO, GO, NT>>(Aij, true);
      RCP<Tpetra::CrsMatrix<ST, LO, GO, NT>> eAij =
          rcp_dynamic_cast<Tpetra::CrsMatrix<ST, LO, GO, NT>>(tAij->getTpetraOperator(), true);

      // rebuild the blocks and place it the right location
      Blocking::rebuildSubBlock(i, j, crsContent, blockMaps_, *eAij, plocal2ContigGIDs[j]);
    }
  }  // end for i
}

}  // namespace TpetraHelpers
}  // end namespace Teko
