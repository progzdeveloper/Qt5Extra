project(widgetdelegatedemo LANGUAGES CXX)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

cmake_path(SET SUBPROJECT_ROOT "${QT5EXTRA_EXAMPLES_ROOT}/${PROJECT_NAME}")
message(STATUS "SUBPROJECT_ROOT=${SUBPROJECT_ROOT}")

add_definitions(-DQTWIDGETSEXTRA_DLL)
include_directories(${QT5EXTRA_ROOT}/qtwidgetsextra/include)

find_sources(SUBPROJECT_SOURCES "${SUBPROJECT_ROOT}" "cpp")
find_sources(SUBPROJECT_HEADERS "${SUBPROJECT_ROOT}" "h")

#add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32 ${SUBPROJECT_SOURCES} ${SUBPROJECT_HEADERS})
add_executable(${PROJECT_NAME} ${SUBPROJECT_SOURCES} ${SUBPROJECT_HEADERS})
add_dependencies(${PROJECT_NAME} qtwidgetsextra)
target_link_libraries(${PROJECT_NAME} qtwidgetsextra Qt5::Core Qt5::Widgets)
