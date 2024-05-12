project(htmlexporter LANGUAGES CXX)

cmake_path(SET SUBPROJECT_ROOT "${QT5EXTRA_MODELEXPORTERS_ROOT}/${PROJECT_NAME}")
message(STATUS "SUBPROJECT_ROOT=${SUBPROJECT_ROOT}")

find_package(Qt5 COMPONENTS Core Widgets Qt5AxContainer REQUIRED)
include_directories(${Qt5AxContainer_INCLUDE_DIRS})
add_definitions(${Qt5AxContainer_DEFINITIONS})
include_directories(${Qt5AxContainer_INCLUDE_DIRS})

add_definitions(-DQTWIDGETSEXTRA_DLL)
include_directories(${QT5EXTRA_ROOT}/qtwidgetsextra/include)

add_definitions(-DQTPROPERTYBROWSER_DLL)
include_directories(${QT5EXTRA_ROOT}/qtpropertybrowser/include)

find_sources(SUBPROJECT_SOURCES "${SUBPROJECT_ROOT}" "cpp")
find_sources(SUBPROJECT_HEADERS "${SUBPROJECT_ROOT}" "h")

#add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32 ${SUBPROJECT_SOURCES} ${SUBPROJECT_HEADERS})
add_library(${PROJECT_NAME} SHARED ${SUBPROJECT_SOURCES} ${SUBPROJECT_HEADERS})

add_dependencies(${PROJECT_NAME} qtwidgetsextra qtpropertybrowser)
target_link_libraries(${PROJECT_NAME} qtwidgetsextra qtpropertybrowser Qt5::Core Qt5::Widgets ${Qt5AxContainer_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${QT5EXTRA_PLUGIN_DIR})

