
INCLUDE("${CTEST_SCRIPT_DIRECTORY}/TrilinosCTestDriverCore.zan.gcc.cmake")

#
# Set the options specific to this build case
#

SET(COMM_TYPE SERIAL)
SET(BUILD_TYPE DEBUG)
SET(BUILD_DIR_NAME SERIAL_DEBUG_DEV)
#SET(CTEST_TEST_TYPE EXPERIMENTAL)
#SET(CTEST_TEST_TIMEOUT 900)

SET(Trilinos_ENABLE_SECONDARY_STABLE_CODE ON)
SET(EXTRA_EXCLUDE_PACKAGES TriKota)

SET( EXTRA_CONFIGURE_OPTIONS
  "-DTrilinos_ENABLE_EXPLICIT_INSTANTIATION:BOOL=ON"
  "-DTrilinos_DATA_DIR:STRING=$ENV{TRILINOSDATADIRECTORY}"
  "-DTPL_ENABLE_Pthread:BOOL=ON"
  "-DNOX_ENABLE_ABSTRACT_IMPLEMENTATION_LAPACK=ON"
  "-DTrilinos_ENABLE_TriKota:BOOL=OFF"
  "-DBoost_INCLUDE_DIRS=/home/trilinos/tpl/gcc4.4.4/boost_1_46_1"
  "-DNetcdf_INCLUDE_DIRS=/home/trilinos/tpl/gcc4.4.4/netcdf-4.1.3/include"
  "-DNetcdf_LIBRARY_DIRS=/home/trilinos/tpl/gcc4.4.4/netcdf-4.1.3/lib"
  )

#
# Set the rest of the system-specific options and run the dashboard build/test
#

TRILINOS_SYSTEM_SPECIFIC_CTEST_DRIVER()