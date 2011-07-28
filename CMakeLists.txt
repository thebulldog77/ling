cmake_minimum_required(VERSION 2.8)
project(WntrLing)

set(WNTRLING_MAJOR_VERSION 0)
set(WNTRLING_MINOR_VERSION 1)
set(WNTRLING_VERSION
    ${WNTRLING_MAJOR_VERSION}.${WNTRLING_MINOR_VERSION})

## Let's allow to choose the installation dirs.
set(LIB_INSTALL_DIR lib CACHE PATH "Installation dir for libraries")
set(INCLUDE_INSTALL_DIR include/wntr/ling CACHE PATH
    "Installation dir for headers")
set(DATA_INSTALL_DIR share CACHE PATH
    "Installation dir for data")

## Here we turn relative paths to absolute.
foreach(p LIB INCLUDE DATA)
    SET(var ${p}_INSTALL_DIR)
    IF(NOT IS_ABSOLUTE "${${var}}")
        set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
    endif()
endforeach()

## Packages
include(FindQt4)
include(FindBoost)
include(FindPythonLibs)
include(WinterDefaults)
include(WinterLibsDependencies)
include(WinterMacros)

find_package(WntrData REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)
find_package(Boost 1.4 COMPONENTS
    python
    REQUIRED)

find_package(Qt4 4.4.3 COMPONENTS
    QtCore
    QT_DONT_USE_GUI
    REQUIRED)

## Includes
include("${QT_USE_FILE}")


set(WNTRLING_INCLUDE_DIR "${WINTER_INCLUDE_INSTALL_DIR}/wntr/ling" CACHE PATH "Includes dir.")
set(WNTRLING_LIB_DIR "${WINTER_LIB_INSTALL_DIR}" CACHE PATH "Libraries dir.")
set(WNTRLING_INCLUDE_DIRS "${WNTRLING_INCLUDE_DIR}"
    "${WNTRDATA_INCLUDE_DIRS}"
    ${PYTHON_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}
    ${QT_INCLUDE_DIR})
set(WNTRLING_CMAKE_DIR "${CMAKE_INSTALL_PREFIX}/share/cmake/WntrLing" CACHE PATH "CMake dir.")
set(WNTRLING_LIBRARIES "${WNTRDATA_LIBRARIES}"
    ${PYTHON_LIBRARIES}
    ${QT_LIBRARIES}
    ${Boost_LIBRARIES})


include_directories(${WNTRLING_INCLUDE_DIRS}
    "include/wntr/ling")

file(GLOB WNTRLING_SHPP include/wntr/ling/*.hpp)
file(GLOB WNTRLING_SCPP src/*.cpp)

#
# Targets
#
add_library(wntrling
    SHARED
    ${WNTRLING_SHPP}
    ${WNTRLING_SCPP})

set_target_properties(wntrling PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(wntrling ${WNTRLING_LIBRARIES})

#
# Configs
#
configure_file("cmake/WntrLingConfig.cmake.in"
    "${PROJECT_BINARY_DIR}/InstallFiles/WntrLingConfig.cmake" @ONLY)

configure_file("cmake/WntrLingConfigVersion.cmake.in"
    "${PROJECT_BINARY_DIR}/InstallFiles/WntrLingConfigVersion.cmake" @ONLY)

configure_file("${PROJECT_SOURCE_DIR}/wntrling.pc.in"
   "${PROJECT_BINARY_DIR}/InstallFiles/wntrling.pc" @ONLY)

#
# Installs
#
install(TARGETS wntrling
    EXPORT WntrLingLibraryDepends
    LIBRARY DESTINATION "${WNTRLING_LIB_DIR}")

install(EXPORT WntrLingLibraryDepends
    DESTINATION "${WNTRLING_CMAKE_DIR}")

install(FILES
    "${PROJECT_BINARY_DIR}/InstallFiles/WntrLingConfig.cmake"
    "${PROJECT_BINARY_DIR}/InstallFiles/WntrLingConfigVersion.cmake"
    DESTINATION "${WNTRLING_CMAKE_DIR}")

install(FILES ${WNTRLING_SHPP}
    DESTINATION "${WNTRLING_INCLUDE_DIR}")

install(FILES ${CMAKE_MODULES}
    DESTINATION "${CMAKE_MODULE_DIR}")

install(FILES "${PROJECT_BINARY_DIR}/InstallFiles/wntrling.pc"
    DESTINATION "${WINTER_LIB_INSTALL_DIR}/pkgconfig/")
