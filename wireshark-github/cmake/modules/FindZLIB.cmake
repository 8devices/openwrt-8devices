# - Find zlib
#
# Find the native ZLIB includes and library.
# Once done this will define
#
#  ZLIB_INCLUDE_DIRS   - where to find zlib.h, etc.
#  ZLIB_LIBRARIES      - List of libraries when using zlib.
#  ZLIB_FOUND          - True if zlib found.
#  ZLIB_DLL_DIR        - (Windows) Path to the zlib DLL.
#  ZLIB_DLL            - (Windows) Name of the zlib DLL.
#
#  ZLIB_VERSION_STRING - The version of zlib found (x.y.z)
#  ZLIB_VERSION_MAJOR  - The major version of zlib
#  ZLIB_VERSION_MINOR  - The minor version of zlib
#  ZLIB_VERSION_PATCH  - The patch version of zlib
#  ZLIB_VERSION_TWEAK  - The tweak version of zlib
#
# The following variable are provided for backward compatibility
#
#  ZLIB_MAJOR_VERSION  - The major version of zlib
#  ZLIB_MINOR_VERSION  - The minor version of zlib
#  ZLIB_PATCH_VERSION  - The patch version of zlib

#=============================================================================
# Copyright 2001-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# We set these manually on Windows.
#INCLUDE(FindWSWinLibs)
#FindWSWinLibs("zlib" "ZLIB_HINTS")

if (NOT ZLIB_INCLUDE_DIR OR NOT ZLIB_LIBRARY)
    find_package(PkgConfig)
    pkg_search_module(ZLIB zlib)

    FIND_PATH(ZLIB_INCLUDE_DIR
        NAMES
            zlib.h
        HINTS
            "${ZLIB_INCLUDEDIR}"
            ${ZLIB_HINTS}/include
            ${ZLIB_HINTS}
        PATHS
            "[HKEY_LOCAL_MACHINE\\SOFTWARE\\GnuWin32\\Zlib;InstallPath]/include"
    )

    SET(ZLIB_NAMES z zlib zdll zlib1 zlibd zlibd1)
    FIND_LIBRARY(ZLIB_LIBRARY
        NAMES
            ${ZLIB_NAMES}
        HINTS
            "${ZLIB_LIBDIR}"
            ${ZLIB_HINTS}/lib
            ${ZLIB_HINTS}
        PATHS
            "[HKEY_LOCAL_MACHINE\\SOFTWARE\\GnuWin32\\Zlib;InstallPath]/lib"
    )
endif()
MARK_AS_ADVANCED(ZLIB_LIBRARY ZLIB_INCLUDE_DIR)

IF(ZLIB_INCLUDE_DIR AND EXISTS "${ZLIB_INCLUDE_DIR}/zlib.h")
    FILE(STRINGS "${ZLIB_INCLUDE_DIR}/zlib.h" ZLIB_H REGEX "^#define ZLIB_VERSION \"[^\"]*\"$")

    STRING(REGEX REPLACE "^.*ZLIB_VERSION \"([0-9]+).*$" "\\1" ZLIB_VERSION_MAJOR "${ZLIB_H}")
    STRING(REGEX REPLACE "^.*ZLIB_VERSION \"[0-9]+\\.([0-9]+).*$" "\\1" ZLIB_VERSION_MINOR  "${ZLIB_H}")
    STRING(REGEX REPLACE "^.*ZLIB_VERSION \"[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" ZLIB_VERSION_PATCH "${ZLIB_H}")
    SET(ZLIB_VERSION_STRING "${ZLIB_VERSION_MAJOR}.${ZLIB_VERSION_MINOR}.${ZLIB_VERSION_PATCH}")

    # only append a TWEAK version if it exists:
    SET(ZLIB_VERSION_TWEAK "")
    IF( "${ZLIB_H}" MATCHES "^.*ZLIB_VERSION \"[0-9]+\\.[0-9]+\\.[0-9]+\\.([0-9]+).*$")
        SET(ZLIB_VERSION_TWEAK "${CMAKE_MATCH_1}")
        SET(ZLIB_VERSION_STRING "${ZLIB_VERSION_STRING}.${ZLIB_VERSION_TWEAK}")
    ENDIF( "${ZLIB_H}" MATCHES "^.*ZLIB_VERSION \"[0-9]+\\.[0-9]+\\.[0-9]+\\.([0-9]+).*$")

    SET(ZLIB_MAJOR_VERSION "${ZLIB_VERSION_MAJOR}")
    SET(ZLIB_MINOR_VERSION "${ZLIB_VERSION_MINOR}")
    SET(ZLIB_PATCH_VERSION "${ZLIB_VERSION_PATCH}")
ENDIF()

#
# Sigh.  On Windows, we build libz as part of the Wireshark build
# process, so we don't necessarily *have* a libz library to search
# for inflatePrime() at this point; the search fails on the buildbots,
# for example.  See bug 13850.
#
# So, on Windows, we just assume we have a new enough version of
# libz, so that it has inflatePrime().
#
IF(WIN32)
    MESSAGE(STATUS "Zlib might not be built yet; assume it contains inflatePrime")
    SET(HAVE_INFLATEPRIME ON)
ELSE()
    INCLUDE(CheckFunctionExists)
    SET(CMAKE_REQUIRED_LIBRARIES ${ZLIB_LIBRARY})
    #
    # Check for inflate() in zlib, to make sure the zlib library is
    # usable.
    #
    # For example, on at least some versions of Fedora, if you have a
    # 64-bit machine, have both the 32-bit and 64-bit versions of the
    # run-time zlib package installed, and have only the *32-bit*
    # version of the zlib development package installed, it'll find the
    # header, and think it can use zlib, and will use it in subsequent
    # tests, but it'll try and link 64-bit test programs with the 32-bit
    # library, causing those tests to falsely fail.  Hilarity ensues.
    #
    CHECK_FUNCTION_EXISTS("inflate" HAVE_INFLATE)
    IF(NOT HAVE_INFLATE)
        MESSAGE(FATAL_ERROR "zlib.h found but linking with -lz failed to find inflate(); do you have the right developer package installed (32-bit vs. 64-bit)?")
    ENDIF()
    CHECK_FUNCTION_EXISTS("inflatePrime" HAVE_INFLATEPRIME)
    # reset
    SET(CMAKE_REQUIRED_LIBRARIES "")
ENDIF()

# handle the QUIETLY and REQUIRED arguments and set ZLIB_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ZLIB REQUIRED_VARS ZLIB_LIBRARY ZLIB_INCLUDE_DIR
                                       VERSION_VAR ZLIB_VERSION_STRING)

IF(ZLIB_FOUND)
    SET(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR})
    SET(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
    #if (WIN32)
    #  set ( ZLIB_DLL_DIR "${ZLIB_HINTS}"
    #    CACHE PATH "Path to the Zlib DLL"
    #  )
    #  file( GLOB _zlib_dll RELATIVE "${ZLIB_DLL_DIR}"
    #    "${ZLIB_DLL_DIR}/zlib1.dll"
    #  )
    #  set ( ZLIB_DLL ${_zlib_dll}
    #    # We're storing filenames only. Should we use STRING instead?
    #    CACHE FILEPATH "Zlib DLL file name"
    #  )
    #  mark_as_advanced( ZLIB_DLL_DIR ZLIB_DLL )
    #endif()
ELSE()
    SET(ZLIB_INCLUDE_DIRS )
    SET(ZLIB_LIBRARIES )
    SET(ZLIB_DLL_DIR )
    SET(ZLIB_DLL )
ENDIF()
