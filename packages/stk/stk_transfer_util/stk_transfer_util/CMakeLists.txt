# Copyright 2002 - 2008, 2010, 2011 National Technology Engineering
# Solutions of Sandia, LLC (NTESS). Under the terms of Contract
# DE-NA0003525 with NTESS, the U.S. Government retains certain rights
# in this software.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
# 
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
# 
#     * Neither the name of NTESS nor the names of its contributors
#       may be used to endorse or promote products derived from this
#       software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 

SET(HEADERS "")
SET(SOURCES "")

FILE(GLOB HEADERS CONFIGURE_DEPENDS *.hpp)
FILE(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)

if(HAVE_STK_Trilinos)
  TRIBITS_INCLUDE_DIRECTORIES(${${PACKAGE_NAME}_SOURCE_DIR})

  TRIBITS_ADD_LIBRARY(stk_transfer_util
    NOINSTALLHEADERS ${HEADERS}
    SOURCES ${SOURCES}
  )
  find_package(LAPACK REQUIRED)
  target_link_libraries(stk_transfer_util PUBLIC ${LAPACK_LIBRARIES})
else()
  add_library(stk_transfer_util ${SOURCES})
  target_link_libraries(stk_transfer_util PUBLIC stk_transfer_lib)
  target_link_libraries(stk_transfer_util PUBLIC stk_search_util)
  target_link_libraries(stk_transfer_util PUBLIC stk_io)
  target_link_libraries(stk_transfer_util PUBLIC stk_util_env)
  target_link_libraries(stk_transfer_util PUBLIC stk_util_command_line)

  if(STK_BUILT_FOR_SIERRA)
    find_package(SierraLapack REQUIRED)
    target_link_libraries(stk_transfer_util PUBLIC SierraLapack::sierra_blas_lapack)
  else()
    find_package(LAPACK REQUIRED)
    target_link_libraries(stk_transfer_util PUBLIC ${LAPACK_LIBRARIES})
    find_package(BLAS REQUIRED)
    target_link_libraries(stk_transfer_util PUBLIC ${BLAS_LIBRARIES})
  endif()

  target_include_directories(stk_transfer_util PUBLIC
   $<BUILD_INTERFACE:${STK_TOPLEVEL_SOURCE_DIR}/stk_transfer_util>
   $<INSTALL_INTERFACE:include>
  )
  INSTALL(TARGETS stk_transfer_util EXPORT stkTargets DESTINATION ${STK_INSTALL_LIBDIR})
endif()

INSTALL(FILES ${HEADERS} DESTINATION
    ${${PROJECT_NAME}_INSTALL_INCLUDE_DIR}/stk_transfer_util)


