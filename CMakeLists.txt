cmake_minimum_required(VERSION 3.15)
project(cgmfwrap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

# CGMF source: use an existing checkout if CGMF_ROOT is given, otherwise fetch
# from GitHub. CGMF is always built from source so that the resulting wheel is
# self-contained.
set(CGMF_ROOT "" CACHE PATH
    "Existing CGMF source checkout. Leave empty to auto-fetch from GitHub.")
set(CGMF_GIT_REPOSITORY "https://github.com/lanl/CGMF.git" CACHE STRING
    "CGMF git repository used when CGMF_ROOT is empty.")
set(CGMF_GIT_TAG "master" CACHE STRING
    "CGMF git tag, branch, or commit used when CGMF_ROOT is empty.")

if(CGMF_ROOT)
    message(STATUS "Using CGMF source from CGMF_ROOT=${CGMF_ROOT}")
    set(_cgmf_source "${CGMF_ROOT}")
else()
    message(STATUS
        "Fetching CGMF from ${CGMF_GIT_REPOSITORY}@${CGMF_GIT_TAG}")
    include(FetchContent)
    FetchContent_Declare(
        cgmf_external
        GIT_REPOSITORY "${CGMF_GIT_REPOSITORY}"
        GIT_TAG        "${CGMF_GIT_TAG}"
        GIT_SHALLOW    TRUE
    )
    FetchContent_GetProperties(cgmf_external)
    if(NOT cgmf_external_POPULATED)
        FetchContent_Populate(cgmf_external)
    endif()
    set(_cgmf_source "${cgmf_external_SOURCE_DIR}")
endif()

# Build CGMF as a PIC-enabled static library so its objects can be linked into
# the pybind11 module. No runtime libcgmf dependency in the installed wheel.
set(cgmf.shared_library OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(
    "${_cgmf_source}"
    "${CMAKE_BINARY_DIR}/_cgmf_build"
    EXCLUDE_FROM_ALL
)

pybind11_add_module(_native cgmf_wrapper.cpp)

target_link_libraries(_native PRIVATE cgmf)

# Stage the cgmfwrap Python package layout into the wheel:
#   cgmfwrap/
#     __init__.py   (from src/cgmfwrap/)
#     _native.so    (this target)
#     data/         (CGMF runtime data files)
install(TARGETS _native LIBRARY DESTINATION cgmfwrap)
install(DIRECTORY "${_cgmf_source}/data/" DESTINATION cgmfwrap/data)
