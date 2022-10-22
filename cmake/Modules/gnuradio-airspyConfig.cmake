find_package(PkgConfig)

PKG_CHECK_MODULES(PC_GR_AIRSPY gnuradio-airspy)

FIND_PATH(
    GR_AIRSPY_INCLUDE_DIRS
    NAMES gnuradio/airspy/api.h
    HINTS $ENV{AIRSPY_DIR}/include
        ${PC_AIRSPY_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GR_AIRSPY_LIBRARIES
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

include("${CMAKE_CURRENT_LIST_DIR}/gnuradio-airspyTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GR_AIRSPY DEFAULT_MSG GR_AIRSPY_LIBRARIES GR_AIRSPY_INCLUDE_DIRS)
MARK_AS_ADVANCED(GR_AIRSPY_LIBRARIES GR_AIRSPY_INCLUDE_DIRS)
