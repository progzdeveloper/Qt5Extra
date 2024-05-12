set(LIB_MAJOR_VERSION "1")
set(LIB_MINOR_VERSION "0")
set(LIB_PATCH_VERSION "0")
set(LIB_VERSION_STRING "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_PATCH_VERSION}")

project(qtlayoutsextra VERSION ${LIB_VERSION_STRING} LANGUAGES CXX)

message(STATUS "${PROJECT_NAME} version=${LIB_VERSION_STRING}")

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

cmake_path(SET SUBPROJECT_ROOT "${QT5EXTRA_ROOT}/${PROJECT_NAME}")
message(STATUS "SUBPROJECT_ROOT=${SUBPROJECT_ROOT}")

add_definitions(-DQTLAYOUTSEXTRA_MAKEDLL -DQTLAYOUTSEXTRA_BUILD_LIB)
find_sources(SUBPROJECT_SOURCES "${SUBPROJECT_ROOT}/src" "cpp")
find_sources(SUBPROJECT_HEADERS "${SUBPROJECT_ROOT}/src" "h")
include_directories(${PROJECT_NAME} "${SUBPROJECT_ROOT}/include")
include_directories(${PROJECT_NAME} "${SUBPROJECT_ROOT}/src/utils")

add_definitions(-DQTGEOMETRY_DLL)
include_directories(${QT5EXTRA_ROOT}/qtgeometry/include)

add_definitions(-DQTCOREEXTRA_DLL)
include_directories(${QT5EXTRA_ROOT}/qtcoreextra/include)

add_library(${PROJECT_NAME} SHARED ${SUBPROJECT_SOURCES} ${SUBPROJECT_HEADERS} ${SUBPROJECT_INTERFACE_HEADER})
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets qtcoreextra qtgeometry)
add_dependencies(${PROJECT_NAME} qtgeometry qtcoreextra)

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_NAME lib${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_NAME_DEBUG lib${PROJECT_NAME}d)
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_NAME ${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_NAME_DEBUG ${PROJECT_NAME}d)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${QT5EXTRA_LIB_DIR})
#set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${QT5EXTRA_ROOT}/libs) # for static linkage
