#!/bin/bash

echo
echo "Starting nightly Trilinos development testing on typhon: `date`"
echo

#
# TrilinosDriver settings:
#

export TDD_PARALLEL_LEVEL=2

# Trilinos settings:
#

# Submission mode for the *TrilinosDriver* dashboard
export TDD_CTEST_TEST_TYPE=Nightly

#export CTEST_DO_SUBMIT=FALSE
#export CTEST_START_WITH_EMPTY_BINARY_DIRECTORY=FALSE

# Machine specific environment
#

source /projects/modulefiles/utils/sems-modules-init.sh
source /projects/modulefiles/utils/kokkos-modules-init.sh

module load python/2.7.9
module load cuda/6.5.14

export FROM_JENKINS=1
export TDD_HTTP_PROXY="http://sonproxy.sandia.gov:80"
export http_proxy="http://sonproxy.sandia.gov:80"
export TDD_FORCE_CMAKE_INSTALL=1
export CUDA_LAUNCH_BLOCKING=1
export OMP_NUM_THREADS=2

# Machine independent cron_driver:
#
#openmpi-1.7-cuda6

SCRIPT_DIR=`cd "\`dirname \"$0\"\`";pwd`

module load intel/15.0.2/openmpi/1.8.7/cuda/6.5.14
module load superlu/4.3/intel/15.0.2/base

cd $WORKSPACE/kokkos && git reset --hard origin/master && cd -
$WORKSPACE/kokkos/config/snapshot.py --verbose $WORKSPACE/kokkos $WORKSPACE/Trilinos/packages
export KOKKOS_BRANCH=master

$SCRIPT_DIR/../cron_driver.py

cd $WORKSPACE/kokkos && git reset --hard origin/develop && cd -
$WORKSPACE/kokkos/config/snapshot.py --verbose $WORKSPACE/kokkos $WORKSPACE/Trilinos/packages
export KOKKOS_BRANCH=develop

$SCRIPT_DIR/../cron_driver.py

module unload intel/15.0.2/openmpi/1.8.7/cuda/6.5.14
module unlaod superlu/4.3/intel/15.0.2/base

module load gcc/4.8.4/openmpi/1.8.7/cuda/6.5.14
module load superlu/4.3/gcc/4.8.4/base

cd $WORKSPACE/kokkos && git reset --hard origin/master && cd -
$WORKSPACE/kokkos/config/snapshot.py --verbose $WORKSPACE/kokkos $WORKSPACE/Trilinos/packages
export KOKKOS_BRANCH=master

$SCRIPT_DIR/../cron_driver.py

cd $WORKSPACE/kokkos && git reset --hard origin/develop && cd -
$WORKSPACE/kokkos/config/snapshot.py --verbose $WORKSPACE/kokkos $WORKSPACE/Trilinos/packages
export KOKKOS_BRANCH=develop

$SCRIPT_DIR/../cron_driver.py

echo
echo "Ending nightly Trilinos development testing on typhon: `date`"
echo