project(WntrLingLib)

file(GLOB WNTRLING_SOURCES "*.cpp")
file(GLOB WNTRLING_HEADERS "*.hpp")
QT4_WRAP_CPP(WNTRLING_SOURCES ${WNTRLING_HEADERS})

add_library(wintermute-plugin-linguistics SHARED
    ${WNTRLING_SOURCES})

set_target_properties(wintermute-plugin-linguistics PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(wintermute-plugin-linguistics ${WNTRLING_LIBRARIES})

install(TARGETS wintermute-plugin-linguistics
    EXPORT WntrLingLibraryDepends
    LIBRARY DESTINATION "${WNTRLING_LIB_DIR}")

install(EXPORT WntrLingLibraryDepends
    DESTINATION "${WNTRLING_CMAKE_DIR}")

install(FILES ${WNTRLING_HEADERS}
    DESTINATION "${WNTRLING_INCLUDE_DIR}"
    COMPONENT dev)
