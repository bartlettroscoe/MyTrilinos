// @HEADER
// *****************************************************************************
//   Zoltan2: A package of combinatorial algorithms for scientific computing
//
// Copyright 2012 NTESS and the Zoltan2 contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

//
// Testing of CoordinateModel
//

#include "Kokkos_UnorderedMap.hpp"
#include <Zoltan2_CoordinateModel.hpp>
#include <Zoltan2_BasicVectorAdapter.hpp>
#include <Zoltan2_TestHelpers.hpp>

#include <set>
#include <bitset>

#include <Teuchos_Comm.hpp>
#include <Teuchos_CommHelpers.hpp>
#include <Teuchos_DefaultComm.hpp>
#include <Teuchos_ArrayView.hpp>
#include <Teuchos_OrdinalTraits.hpp>

#include <Tpetra_CrsMatrix.hpp>

using Teuchos::RCP;
using Teuchos::Comm;

void testCoordinateModel(std::string &fname, int nWeights,
  const RCP<const Comm<int> > &comm,
  bool nodeZeroHasAll, bool printInfo)
{
  int fail = 0, gfail = 0;

  if (printInfo){
    std::cout << "Test: " << fname << std::endl;
    std::cout << "Num Weights: " << nWeights;
    std::cout << " proc 0 has all: " << nodeZeroHasAll;
    std::cout << std::endl;
  }

  //////////////////////////////////////////////////////////////
  // Input data
  //////////////////////////////////////////////////////////////

  typedef Tpetra::MultiVector<zscalar_t, zlno_t, zgno_t, znode_t> mv_t;

  RCP<UserInputForTests> uinput;

  try{
    uinput = rcp(new UserInputForTests(testDataFilePath, fname, comm, true));
  }
  catch(std::exception &e){
    fail=1;
  }

  TEST_FAIL_AND_EXIT(*comm, !fail, "input constructor", 1);

  RCP<mv_t> coords;

  try{
    coords = uinput->getUICoordinates();
  }
  catch(std::exception &e){
    fail=2;
  }

  TEST_FAIL_AND_EXIT(*comm, !fail, "getting coordinates", 1);

  int coordDim = coords->getNumVectors();

  TEST_FAIL_AND_EXIT(*comm, coordDim <= 3, "dim 3 at most", 1);

  const zscalar_t *x=NULL, *y=NULL, *z=NULL;

  x = coords->getData(0).getRawPtr();
  if (coordDim > 1){
    y = coords->getData(1).getRawPtr();
    if (coordDim > 2)
      z = coords->getData(2).getRawPtr();
  }

  // Are these coordinates correct

  int nLocalIds = coords->getLocalLength();
  ArrayView<const zgno_t> idList = coords->getMap()->getLocalElementList();

  int nGlobalIds = 0;
  if (nodeZeroHasAll){
    if (comm->getRank() > 0){
      x = y = z = NULL;
      nLocalIds = 0;
    }
    else{
      nGlobalIds = nLocalIds;
    }
    Teuchos::broadcast<int, int>(*comm, 0, &nGlobalIds);
  }
  else{
    nGlobalIds = coords->getGlobalLength();
  }

  Array<ArrayRCP<const zscalar_t> > coordWeights(nWeights);

  if (nLocalIds > 0){
    for (int wdim=0; wdim < nWeights; wdim++){
      zscalar_t *w = new zscalar_t [nLocalIds];
      for (int i=0; i < nLocalIds; i++){
        w[i] = ((i%2) + 1) + wdim;
      }
      coordWeights[wdim] = Teuchos::arcp(w, 0, nLocalIds);
    }
  }


  //////////////////////////////////////////////////////////////
  // Create a BasicVectorAdapter adapter object.
  //////////////////////////////////////////////////////////////

  typedef Zoltan2::BasicVectorAdapter<mv_t> ia_t;
  typedef Zoltan2::VectorAdapter<mv_t> base_ia_t;

  RCP<ia_t> ia;

  if (nWeights == 0){   // use the simpler constructor
    try{
      ia = rcp(new ia_t(nLocalIds, idList.getRawPtr(), x, y, z));
    }
    catch(std::exception &e){
      fail=3;
    }
  }
  else{
    std::vector<const zscalar_t *> values, weights;
    std::vector<int> valueStrides, weightStrides;  // default is 1
    values.push_back(x);
    if (y) {
      values.push_back(y);
      if (z)
        values.push_back(z);
    }
    for (int wdim=0; wdim < nWeights; wdim++){
      weights.push_back(coordWeights[wdim].getRawPtr());
    }

    try{
      ia = rcp(new ia_t(nLocalIds, idList.getRawPtr(),
               values, valueStrides, weights, weightStrides));
    }
    catch(std::exception &e){
      fail=4;
    }
  }

  RCP<const base_ia_t> base_ia = Teuchos::rcp_dynamic_cast<const base_ia_t>(ia);

  TEST_FAIL_AND_EXIT(*comm, !fail, "making input adapter", 1);

  //////////////////////////////////////////////////////////////
  // Create an CoordinateModel with this input
  //////////////////////////////////////////////////////////////

  typedef Zoltan2::StridedData<zlno_t, zscalar_t> input_t;
  typedef std::bitset<Zoltan2::NUM_MODEL_FLAGS> modelFlags_t;
  typedef Zoltan2::CoordinateModel<base_ia_t> model_t;
  modelFlags_t modelFlags;

  RCP<const Zoltan2::Environment> env = rcp(new Zoltan2::Environment(comm));
  RCP<model_t> model;


  try{
    model = rcp(new model_t(base_ia, env, comm, modelFlags));
  }
  catch (std::exception &e){
    fail = 5;
  }

  TEST_FAIL_AND_EXIT(*comm, !fail, "making model", 1);

  // Test the CoordinateModel interface

  if (model->getCoordinateDim() != coordDim)
    fail = 6;

  if (!fail && model->getLocalNumCoordinates() != size_t(nLocalIds))
    fail = 7;

  if (!fail && model->getGlobalNumCoordinates() != size_t(nGlobalIds))
    fail = 8;

  if (!fail && model->getNumWeightsPerCoordinate() !=  nWeights)
    fail = 9;

  gfail = globalFail(*comm, fail);

  if (gfail)
    printFailureCode(*comm, fail);

  ArrayView<const zgno_t> gids;
  ArrayView<input_t> xyz;
  ArrayView<input_t> wgts;

  model->getCoordinates(gids, xyz, wgts);

  if (!fail && gids.size() != nLocalIds)
    fail = 10;

  for (int i=0; !fail && i < nLocalIds; i++){
    if (gids[i] != idList[i])
      fail = 11;
  }

  if (!fail && wgts.size() != nWeights)
    fail = 12;

  const zscalar_t *vals[3] = {x, y, z};

  for (int dim=0; !fail && dim < coordDim; dim++){
    for (int i=0; !fail && i < nLocalIds; i++){
      if (xyz[dim][i] != vals[dim][i])
        fail = 13;
    }
  }

  for (int wdim=0; !fail && wdim < nWeights; wdim++){
    for (int i=0; !fail && i < nLocalIds; i++){
      if (wgts[wdim][i] != coordWeights[wdim][i])
        fail = 14;
    }
  }

  gfail = globalFail(*comm, fail);

  if (gfail)
    printFailureCode(*comm, fail);


  ////////////////////////////////
  //////////// KOKKOS ////////////
  ////////////////////////////////
  Kokkos::View<const zgno_t *, typename znode_t::device_type> gidsKokkos;

  Kokkos::View<zscalar_t **, Kokkos::LayoutLeft, typename znode_t::device_type> xyzKokkos;
  Kokkos::View<zscalar_t **, typename znode_t::device_type> wgtsKokkos;

  model->getCoordinatesKokkos(gidsKokkos, xyzKokkos, wgtsKokkos);

  if (!fail && gidsKokkos.extent(0) != static_cast<size_t>(nLocalIds))
    fail = 10;

  auto gidsKokkos_host = Kokkos::create_mirror_view(gidsKokkos);
  Kokkos::deep_copy(gidsKokkos_host, gidsKokkos);

  for (int i=0; !fail && i < nLocalIds; i++){
    if (gidsKokkos_host(i) != idList[i])
      fail = 11;
  }

  if (!fail && wgtsKokkos.extent(1) != static_cast<size_t>(nWeights))
    fail = 12;

  auto xyzKokkos_host = Kokkos::create_mirror_view(xyzKokkos);
  Kokkos::deep_copy(xyzKokkos_host, xyzKokkos);

  for (int dim=0; !fail && dim < coordDim; dim++){
    for (int i=0; !fail && i < nLocalIds; i++){
      if (xyzKokkos_host(i, dim) != vals[dim][i])
        fail = 13;
    }
  }

  auto wgtsKokkos_host = Kokkos::create_mirror_view(wgtsKokkos);
  Kokkos::deep_copy(wgtsKokkos_host, wgtsKokkos);

  for (int wdim=0; !fail && wdim < nWeights; wdim++){
    for (int i=0; !fail && i < nLocalIds; i++){
      if (wgtsKokkos_host(i, wdim) != coordWeights[wdim][i])
        fail = 14;
    }
  }

  gfail = globalFail(*comm, fail);

  if (gfail)
    printFailureCode(*comm, fail);
}

int main(int narg, char *arg[])
{
  Tpetra::ScopeGuard tscope(&narg, &arg);
  Teuchos::RCP<const Teuchos::Comm<int> > comm = Tpetra::getDefaultComm();

  int rank = comm->getRank();
  string fname("simple");   // reader will seek coord file

  testCoordinateModel(fname, 0, comm, false, rank==0);

  testCoordinateModel(fname, 1, comm, false, rank==0);

  testCoordinateModel(fname, 2, comm, false, rank==0);

  testCoordinateModel(fname, 0, comm, true, rank==0);

  testCoordinateModel(fname, 1, comm, true, rank==0);

  testCoordinateModel(fname, 2, comm, true, rank==0);

  if (rank==0) std::cout << "PASS" << std::endl;

  return 0;
}
