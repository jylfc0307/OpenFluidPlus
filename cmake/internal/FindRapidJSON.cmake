##
#
#  This file is part of OpenFLUID software
#  Copyright(c) 2007, INRA - Montpellier SupAgro
#
#
# == GNU General Public License Usage ==
#
#  OpenFLUID is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  OpenFLUID is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with OpenFLUID. If not, see <http://www.gnu.org/licenses/>.
#
#
# == Other Usage ==
#
#  Other Usage means a use of OpenFLUID that is inconsistent with the GPL
#  license, and requires a written agreement between You and INRA.
#  Licensees for Other Usage of OpenFLUID may use this file in accordance
#  with the terms contained in the written agreement between You and INRA.
#  
##
#
#
# Module for CMake build system, helping to find RapidJSON header-only library
#
# This module provides the following variables
# RapidJSON_FOUND 
# RapidJSON_INCLUDE_DIR
# RapidJSON_INCLUDE_DIRS
#
# This module uses hints about search locations from the following paths and variables
# ${CMAKE_BINARY_DIR}/3rdparty/rapidjson
# RapidJSON_EXTRA_SEARCH_DIRS
#
# If RapidJSON headers are not found, this modules tries to clone the RapidJSON git repository,
# with the URL given in the RapidJSON_GIT_URL variable
#
# Examples of use:
#
# FIND_PACKAGE(RapidJSON)
#
##


CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9)

IF(RapidJSON_FOUND AND RapidJSON_INCLUDE_DIR)
  RETURN()
ENDIF()


###########################################################################


SET(_RapidJSON_FOUND 0)

SET(_RapidJSON_CLONE_DIR "${CMAKE_BINARY_DIR}/3rdparty/rapidjson")

SET(_RapidJSON_SEARCH_DIRS
  ${_RapidJSON_CLONE_DIR}
  /usr
  /usr/local
  /sw # Fink
  /opt/local # DarwinPorts
  ${RapidJSON_EXTRA_SEARCH_PATHS}
)


###########################################################################


MACRO(_RapidJSON_FIND_HEADERS)
  FIND_PATH(_RapidJSON_INCLUDE_DIR
    NAMES
      rapidjson/rapidjson.h
    HINTS
      ${_RapidJSON_SEARCH_DIRS}
    PATH_SUFFIXES
      include
  )
ENDMACRO()  


###########################################################################


_RapidJSON_FIND_HEADERS()


IF(NOT _RapidJSON_INCLUDE_DIR)

  MESSAGE(STATUS "RapidJSON not found locally, trying to retreive it from git repository")
  IF(RapidJSON_GIT_URL) # try to retreive from git url
    FIND_PACKAGE(Git QUIET)
    IF(GIT_FOUND)      
      EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} "clone" "${RapidJSON_GIT_URL}" "${_RapidJSON_CLONE_DIR}")
    ELSE()
      MESSAGE(FATAL_ERROR "RapidJSON not found (no git command found for automatic retreiving)")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR "RapidJSON not found (no git repository URL provided for automatic retreiving)")  
  ENDIF()
 
  _RapidJSON_FIND_HEADERS()
 
ENDIF()



IF(_RapidJSON_INCLUDE_DIR)
  SET(_RapidJSON_FOUND 1)
ENDIF()


###########################################################################


IF(_RapidJSON_FOUND)
  MESSAGE(STATUS "Found RapidJSON headers")

  SET(RapidJSON_INCLUDE_DIR ${_RapidJSON_INCLUDE_DIR} CACHE PATH "RapidJSON include directory")
  SET(RapidJSON_INCLUDE_DIRS ${_RapidJSON_INCLUDE_DIR} CACHE PATH "RapidJSON include directories")
     
  SET(RapidJSON_FOUND 1 CACHE STRING "RapidJSON found!")
ELSE()
  MESSAGE(FATAL_ERROR "RapidJSON headers not found")  
ENDIF()


