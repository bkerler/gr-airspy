# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT airspy_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT airspy_sources)

########################################################################
# Check for pygccxml
########################################################################
GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
    )

include(GrPybind)

########################################################################
# Python Bindings
########################################################################

list(APPEND airspy_python_files
    airspyhf_python.cc
    python_bindings.cc)

GR_PYBIND_MAKE_OOT(airspy
   ../../..
   gr::airspy
   "${airspy_python_files}")

# copy bindings extension for use in QA test module
add_custom_command(TARGET airspy_python POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:airspy_python>
    ${PROJECT_BINARY_DIR}/test_modules/gnuradio/airspy/
)

install(TARGETS airspy_python DESTINATION ${GR_PYTHON_DIR}/gnuradio/airspy COMPONENT pythonapi)
