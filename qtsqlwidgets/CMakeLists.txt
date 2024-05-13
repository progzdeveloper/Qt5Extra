set(LIB_MAJOR_VERSION "1")
set(LIB_MINOR_VERSION "0")
set(LIB_PATCH_VERSION "0")
set(LIB_VERSION_STRING "${LIB_MAJOR_VERSION}.${LIB_MINOR_VERSION}.${LIB_PATCH_VERSION}")

set(ENABLE_QTSQLEXRA ON) # todo: move to config
set(ENABLE_QTWIDGETSEXTRA ON)# todo: move to config

project(qtsqlwidgets VERSION ${LIB_VERSION_STRING} LANGUAGES CXX)

message(STATUS "${PROJECT_NAME} version=${LIB_VERSION_STRING}")

find_package(Qt5 COMPONENTS Core Sql Widgets REQUIRED)

cmake_path(SET SUBPROJECT_ROOT "${QT5EXTRA_ROOT}/${PROJECT_NAME}")
message(STATUS "SUBPROJECT_ROOT=${SUBPROJECT_ROOT}")

add_definitions(-DQTSQLWIDGETS_MAKEDLL -DQTSQLWIDGETS_BUILD_LIB)
find_sources(SUBPROJECT_SOURCES "${SUBPROJECT_ROOT}/src" "cpp")
find_sources(SUBPROJECT_HEADERS "${SUBPROJECT_ROOT}/src" "h")
include_directories(${PROJECT_NAME} "${SUBPROJECT_ROOT}/include")

set(QT5EXTRALIBS)

if (ENABLE_QTWIDGETSEXTRA)
    add_definitions(-DQTWIDGETSEXTRA_DLL)
    include_directories(${QT5EXTRA_ROOT}/qtwidgetsextra/include)
    set(QT5EXTRALIBS ${QT5EXTRALIBS} qtwidgetsextra)
endif()

if (ENABLE_QTSQLEXRA)
    add_definitions(-DQTSQLEXTRA_DLL)
    include_directories(${QT5EXTRA_ROOT}/qtsqlextra/include)
    set(QT5EXTRALIBS ${QT5EXTRALIBS} qtsqlextra)
endif()

add_library(${PROJECT_NAME} SHARED ${SUBPROJECT_SOURCES} ${SUBPROJECT_HEADERS} ${SUBPROJECT_INTERFACE_HEADER} ${PROJECT_NAME}.qrc)
target_link_libraries(${PROJECT_NAME} ${QT5EXTRALIBS} Qt5::Core Qt5::Sql Qt5::Widgets)

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_NAME lib${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_NAME_DEBUG lib${PROJECT_NAME}d)
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_NAME ${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_NAME_DEBUG ${PROJECT_NAME}d)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${QT5EXTRA_LIB_DIR})
#set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${QT5EXTRA_ROOT}/libs) # for static linkage
