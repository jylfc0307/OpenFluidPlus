SET(CPACK_GENERATOR "NSIS")
SET(CPACK_SOURCE_GENERATOR "ZIP")
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/resources/packaging;${CMAKE_MODULE_PATH}")        

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "OpenFLUID-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
  
SET(CPACK_PACKAGE_NAME "OpenFLUID")
  
SET(CPACK_NSIS_DISPLAY_NAME "OpenFLUID")
SET(CPACK_NSIS_HELP_LINK "http://www.openfluid-project.org/")
SET(CPACK_NSIS_URL_INFO_ABOUT "http://www.openfluid-project.org/")
SET(CPACK_NSIS_CONTACT "OpenFLUID contact <contact@openfluid-project.org>")  
SET(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/resources/packaging\\\\OF_logo.bmp")  
SET(CPACK_NSIS_MENU_LINKS 
    "http://www.watsave.cn//" "OpenFLUID web site"
    "http://www.openfluid-project.org/community/" "OpenFLUID community site")
SET(CPACK_PACKAGE_EXECUTABLES openfluid-builder;OpenFLUID-Builder
                              openfluid-devstudio;OpenFLUID-DevStudio)   
   
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_NAME} ${FULL_VERSION}")
SET(CPACK_NSIS_MODIFY_PATH ON)


# This packaging includes Qt libs, GEOS libs and GDAL libs
# It requires the SUPPORT_DIR environment variable
FILE(TO_CMAKE_PATH "$ENV{SUPPORT_DIR}" SUPPORT_DIR_MOD)
FILE(TO_CMAKE_PATH "$ENV{SUPPORT_DIR}/bin" SUPPORT_BINDIR_MOD)
FILE(TO_CMAKE_PATH "$ENV{SUPPORT_DIR}/lib" SUPPORT_LIBDIR_MOD)

 # support libraries
IF(NOT SUPPORT_DIR_MOD)
  MESSAGE(FATAL_ERROR "environment variable SUPPORT_DIR is not set!")
ENDIF()


# Qt
SET(WINDEPLOYQT_COMMAND "windeployqt")

SET(WINDEPLOYQT_TARGETS openfluid)

IF(OPENFLUID_ENABLE_GUI)
  IF(BUILD_APP_BUILDER)
    LIST(APPEND WINDEPLOYQT_TARGETS openfluid-builder)
  ENDIF()
  IF(BUILD_APP_DEVSTUDIO)
    LIST(APPEND WINDEPLOYQT_TARGETS openfluid-devstudio)
  ENDIF()
  
ENDIF()

FOREACH(CURRENT_TARGET ${WINDEPLOYQT_TARGETS})
  GET_TARGET_PROPERTY(CURRENT_TARGET_PATH ${CURRENT_TARGET} LOCATION)
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${WINDEPLOYQT_COMMAND} 
                                        ${CURRENT_TARGET_PATH}
                                        --dir \"\${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_PATH}\" 
                                        --libdir \"\${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_PATH}\"
                                        --release --compiler-runtime
                                        -xml -network -concurrent -declarative
                                        --verbose 1
                                WORKING_DIRECTORY \"\${CMAKE_BINARY_DIR}\"
                                OUTPUT_FILE windeployqt_exec_${CURRENT_TARGET}.log 
                                ERROR_FILE windeployqt_exec_${CURRENT_TARGET}.err)")
ENDFOREACH()                     


# GDAL  
INSTALL(DIRECTORY "${SUPPORT_BINDIR_MOD}/" DESTINATION ${BIN_INSTALL_PATH} FILES_MATCHING PATTERN "libgdal*.dll") 
INSTALL(DIRECTORY "${SUPPORT_LIBDIR_MOD}/" DESTINATION ${BIN_INSTALL_PATH} FILES_MATCHING PATTERN "libgdal*.dll") 


# GEOS
IF(OPENFLUID_ENABLE_LANDR)
  INSTALL(DIRECTORY "${SUPPORT_BINDIR_MOD}/" DESTINATION ${BIN_INSTALL_PATH} FILES_MATCHING PATTERN "libgeos*.dll") 
  INSTALL(DIRECTORY "${SUPPORT_LIBDIR_MOD}/" DESTINATION ${BIN_INSTALL_PATH} FILES_MATCHING PATTERN "libgeos*.dll") 
ENDIF()

  
  
