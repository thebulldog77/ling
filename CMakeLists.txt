project(WntrLingApi)
cmake_minimum_required(VERSION 2.8)

set(WINTERMUTE_IS_PLUGIN ON)
set(WNTRLING_MAJOR_VERSION 0)
set(WNTRLING_MINOR_VERSION 1)
set(WNTRLING_UUID "81da3bd6-bed5-4c74-aae6-44f48cd5330c")
set(WNTRLING_VERSION
    ${WNTRLING_MAJOR_VERSION}.${WNTRLING_MINOR_VERSION})

## Packages
find_package(Wintermute 0.1 REQUIRED)
find_package(WntrData 0.1 REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)
find_package(Boost 1.46 REQUIRED)
find_package(Qt4 4.4.3 REQUIRED)

## Includes
include("${QT_USE_FILE}")
include(WintermuteMacros)
include(WintermuteDefaults)

## Variables
set(WNTRLING_INCLUDE_DIR "${WINTER_PLUGIN_INCLUDE_INSTALL_DIR}/ling" CACHE PATH "Includes directory.")
set(WNTRLING_LIB_DIR "${WINTER_PLUGIN_INSTALL_DIR}" CACHE PATH "Libraries directory.")
set(WNTRLING_CMAKE_DIR "${CMAKE_INSTALL_PREFIX}/share/cmake/WntrLing" CACHE PATH "CMake directory.")
set(WNTRLING_INCLUDE_DIRS
        ${WNTRLING_INCLUDE_DIR}
        ${WINTERMUTE_INCLUDE_DIRS}
        ${WNTRDATA_INCLUDE_DIRS}
        ${PYTHON_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${QT_INCLUDE_DIRS})
set(WNTRLING_LIBRARIES
	${WNTRDATA_LIBRARIES}
	${WINTERMUTE_LIBRARIES}
	${PYTHON_LIBRARIES}
	${QT_LIBRARIES}
	${Boost_LIBRARIES})
	
## Configs
configure_file("cmake/WntrLingConfig.cmake.in"
	"${PROJECT_BINARY_DIR}/InstallFiles/WntrLingConfig.cmake" @ONLY)

configure_file("cmake/WntrLingConfigVersion.cmake.in"
        "${PROJECT_BINARY_DIR}/InstallFiles/WntrLingConfigVersion.cmake" @ONLY)

configure_file("${PROJECT_SOURCE_DIR}/spec.in"
	 "${PROJECT_BINARY_DIR}/InstallFiles/${WNTRLING_UUID}.spec" @ONLY)

include_directories(${WNTRLING_INCLUDE_DIRS})
add_subdirectory(src)

## Installs
install(FILES
        "${PROJECT_BINARY_DIR}/InstallFiles/WntrLingConfig.cmake"
        "${PROJECT_BINARY_DIR}/InstallFiles/WntrLingConfigVersion.cmake"
        DESTINATION "${WNTRLING_CMAKE_DIR}")

install(FILES "${PROJECT_BINARY_DIR}/InstallFiles/${WNTRLING_UUID}.spec"
	DESTINATION "${WINTER_PLUGINSPEC_INSTALL_DIR}")
