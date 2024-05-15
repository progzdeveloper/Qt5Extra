project(cld2plugin LANGUAGES CXX)

find_package(Qt5 COMPONENTS Core REQUIRED)

find_package(PkgConfig)
pkg_check_modules(PKG_CLD2 QUIET cld2)
find_path(CLD2_INCLUDE_DIRS
          NAMES public/compact_lang_det.h public/encodings.h
          PATH_SUFFIXES cld2
          HINTS ${PKG_CLD2_INCLUDE_DIRS})
find_library(CLD2_LIBRARIES
             NAMES ${PKG_CLD2_LIBRARIES} cld2 libcld2
             HINTS ${PKG_CLD2_LIBRARY_DIRS})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CLD2
                                  REQUIRED_VARS CLD2_LIBRARIES CLD2_INCLUDE_DIRS
                                  VERSION_VAR PKG_CLD2_VERSION)
mark_as_advanced(CLD2_INCLUDE_DIRS CLD2_LIBRARIES)
message(STATUS "PKG_CLD2_VERSION=${PKG_CLD2_VERSION}")

cmake_path(SET SUBPROJECT_ROOT "${QT5EXTRA_LANGDETECT_ROOT}/${PROJECT_NAME}")
message(STATUS "SUBPROJECT_ROOT=${SUBPROJECT_ROOT}")

add_definitions(-DQTTEXTEXTRA_DLL)
include_directories(${QT5EXTRA_ROOT}/qttextextra/include)

find_sources(SUBPROJECT_SOURCES "${SUBPROJECT_ROOT}" "cpp")
find_sources(SUBPROJECT_HEADERS "${SUBPROJECT_ROOT}" "h")

#add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32 ${SUBPROJECT_SOURCES} ${SUBPROJECT_HEADERS})
add_library(${PROJECT_NAME} SHARED ${SUBPROJECT_SOURCES} ${SUBPROJECT_HEADERS})

add_dependencies(${PROJECT_NAME} qttextextra)
target_link_libraries(${PROJECT_NAME} qttextextra ${CLD2_LIBRARIES} Qt5::Core)
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${QT5EXTRA_PLUGIN_DIR})


