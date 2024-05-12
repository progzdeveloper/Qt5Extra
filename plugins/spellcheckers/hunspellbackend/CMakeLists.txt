project(hunspellbackend LANGUAGES CXX)

find_package(Qt5 COMPONENTS Core REQUIRED)

find_package(PkgConfig)
pkg_check_modules(PKG_HUNSPELL QUIET hunspell)
find_path(HUNSPELL_INCLUDE_DIRS
          NAMES hunspell.hxx
          PATH_SUFFIXES hunspell
          HINTS ${PKG_HUNSPELL_INCLUDE_DIRS})
find_library(HUNSPELL_LIBRARIES
             NAMES ${PKG_HUNSPELL_LIBRARIES} hunspell hunspell-1.3 hunspell-1.8 hunspell-1.7 hunspell-1.6 libhunspell
             HINTS ${PKG_HUNSPELL_LIBRARY_DIRS})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HUNSPELL
                                  REQUIRED_VARS HUNSPELL_LIBRARIES HUNSPELL_INCLUDE_DIRS
                                  VERSION_VAR PKG_HUNSPELL_VERSION)
mark_as_advanced(HUNSPELL_INCLUDE_DIRS HUNSPELL_LIBRARIES)
message(STATUS "PKG_HUNSPELL_VERSION=${PKG_HUNSPELL_VERSION}")

cmake_path(SET SUBPROJECT_ROOT "${QT5EXTRA_SPELLCHECKERS_ROOT}/${PROJECT_NAME}")
message(STATUS "SUBPROJECT_ROOT=${SUBPROJECT_ROOT}")

add_definitions(-DQTSPELLCHECKING_DLL -DLIBHUNSPELL_VERSION=130)
include_directories(${QT5EXTRA_ROOT}/qtspellchecking/include)

find_sources(SUBPROJECT_SOURCES "${SUBPROJECT_ROOT}" "cpp")
find_sources(SUBPROJECT_HEADERS "${SUBPROJECT_ROOT}" "h")

#add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32 ${SUBPROJECT_SOURCES} ${SUBPROJECT_HEADERS})
add_library(${PROJECT_NAME} SHARED ${SUBPROJECT_SOURCES} ${SUBPROJECT_HEADERS})

add_dependencies(${PROJECT_NAME} qtspellchecking)
target_link_libraries(${PROJECT_NAME} qtspellchecking ${HUNSPELL_LIBRARIES} Qt5::Core)
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${QT5EXTRA_PLUGIN_DIR})
