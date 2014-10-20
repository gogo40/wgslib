# - Find the JSONRPC library
#
# Usage:
#   find_package(FFTW [REQUIRED] [QUIET] )
#     
# It sets the following variables:
#   JSONRPC_FOUND               ... true if fftw is found on the system
#   JSONRPC_LIBRARIES           ... full path to fftw library
#   JSONRPC_INCLUDES            ... fftw include directory
#
# The following variables will be checked by the function
#   JSONRPC_USE_STATIC_LIBS    ... if true, only static libraries are found
#   JSONRPC_ROOT               ... if set, the libraries are exclusively searched
#                               under this path
#   JSONRPC_LIBRARY            ... fftw library to use
#   JSONRPC_INCLUDE_DIR        ... fftw include directory
#

#If environment variable FFTWDIR is specified, it has same effect as FFTW_ROOT
if( NOT JSONRPC_ROOT AND ENV{JSONRPCDIR} )
  set( JSONRPC_ROOT $ENV{JSONRPCDIR} )
endif()

# Check if we can use PkgConfig
find_package(PkgConfig)

#Determine from PKG
if( PKG_CONFIG_FOUND AND NOT JSONRPC_ROOT )
  pkg_check_modules( PKG_FFTW QUIET "jsonrpc" )
endif()

#Check whether to search static or dynamic libs
set( CMAKE_FIND_LIBRARY_SUFFIXES_SAV ${CMAKE_FIND_LIBRARY_SUFFIXES} )

if( ${JSONRPC_USE_STATIC_LIBS} )
  set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX} )
else()
  set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX} )
endif()

if( JSONRPC_ROOT )

  #find libs
  find_library(
    JSONRPC_COMMON_LIB
    NAMES "jsonrpccpp-common"
    PATHS ${JSONRPC_ROOT}
    PATH_SUFFIXES "lib" "lib64"
    NO_DEFAULT_PATH
  )

  find_library(
    JSONRPC_CLIENT_LIB
    NAMES "jsonrpccpp-client"
    PATHS ${JSONRPC_ROOT}
    PATH_SUFFIXES "lib" "lib64"
    NO_DEFAULT_PATH
  )

  find_library(
    JSONRPC_SERVER_LIB
    NAMES "jsonrpccpp-server"
    PATHS ${JSONRPC_ROOT}
    PATH_SUFFIXES "lib" "lib64"
    NO_DEFAULT_PATH
  )

  
  #find includes
  find_path(
    JSONRPC_INCLUDES
    NAMES "jsonrpccpp/rpc.h"
    PATHS ${JSONRPC_ROOT}
    PATH_SUFFIXES "include"
    NO_DEFAULT_PATH
  )

else()

  find_library(
    JSONRPC_COMMON_LIB
    NAMES "jsonrpccpp-common"
    PATHS ${PKG_JSONRPCCPP_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
  )

  find_library(
    JSONRPC_CLIENT_LIB
    NAMES "jsonrpccpp-client"
    PATHS ${PKG_JSONRPCCPP_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
  )

  find_library(
    JSONRPC_SERVER_LIB
    NAMES "jsonrpccpp-server"
    PATHS ${PKG_JSONRPCCPP_LIBRARY_DIRS} ${LIB_INSTALL_DIR}
  )

  find_path(
    JSONRPC_INCLUDES
    NAMES "jsonrpccpp/rpc.h"
    PATHS ${PKG_JSONRPCCPP_INCLUDE_DIRS} ${INCLUDE_INSTALL_DIR}
  )

endif( JSONRPC_ROOT )

set(JSONRPC_LIBRARIES ${JSONRPC_COMMON_LIB} ${JSONRPC_CLIENT_LIB} ${JSONRPC_SERVER_LIB})

set( CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES_SAV} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JSONRPC DEFAULT_MSG
                                  JSONRPC_INCLUDES JSONRPC_LIBRARIES)

mark_as_advanced(JSONRPC_INCLUDES JSONRPC_LIBRARIES)

