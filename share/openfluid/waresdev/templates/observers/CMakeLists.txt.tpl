CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9)

INCLUDE(CMake.in.config)

FIND_PACKAGE(OpenFLUIDHelpers REQUIRED)

OPENFLUID_ADD_OBSERVER(OBS)
