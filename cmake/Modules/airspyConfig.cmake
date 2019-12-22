INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_AIRSPY airspy)

FIND_PATH(
    AIRSPY_INCLUDE_DIRS
    NAMES airspy/api.h
    HINTS $ENV{AIRSPY_DIR}/include
        ${PC_AIRSPY_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    AIRSPY_LIBRARIES
    NAMES gnuradio-airspy
    HINTS $ENV{AIRSPY_DIR}/lib
        ${PC_AIRSPY_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/airspyTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(AIRSPY DEFAULT_MSG AIRSPY_LIBRARIES AIRSPY_INCLUDE_DIRS)
MARK_AS_ADVANCED(AIRSPY_LIBRARIES AIRSPY_INCLUDE_DIRS)
