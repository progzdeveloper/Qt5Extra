project(enchantbackend LANGUAGES CXX)

find_package(Qt5 COMPONENTS Core REQUIRED)

find_package(PkgConfig)
pkg_check_modules(PKG_ENCHANT QUIET enchant)
find_path(ENCHANT_INCLUDE_DIRS
          NAMES enchant.h enchant++.h
          PATH_SUFFIXES enchant
          HINTS ${PKG_ENCHANT_INCLUDE_DIRS})
find_library(ENCHANT_LIBRARIES
             NAMES ${PKG_ENCHANT_LIBRARIES} enchant libenchant
             HINTS ${PKG_ENCHANT_LIBRARY_DIRS})
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ENCHANT
                                  REQUIRED_VARS ENCHANT_LIBRARIES ENCHANT_INCLUDE_DIRS
                                  VERSION_VAR PKG_ENCHANT_VERSION)
mark_as_advanced(ENCHANT_INCLUDE_DIRS ENCHANT_LIBRARIES)
message(STATUS "PKG_ENCHANT_VERSION=${PKG_ENCHANT_VERSION}")

cmake_path(SET SUBPROJECT_ROOT "${QT5EXTRA_SPELLCHECKERS_ROOT}/${PROJECT_NAME}")
message(STATUS "SUBPROJECT_ROOT=${SUBPROJECT_ROOT}")

add_definitions(-DQTSPELLCHECKING_DLL)
include_directories(${QT5EXTRA_ROOT}/qtspellchecking/include)

find_sources(SUBPROJECT_SOURCES "${SUBPROJECT_ROOT}" "cpp")
find_sources(SUBPROJECT_HEADERS "${SUBPROJECT_ROOT}" "h")

#add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32 ${SUBPROJECT_SOURCES} ${SUBPROJECT_HEADERS})
add_library(${PROJECT_NAME} SHARED ${SUBPROJECT_SOURCES} ${SUBPROJECT_HEADERS})

add_dependencies(${PROJECT_NAME} qtspellchecking)
target_link_libraries(${PROJECT_NAME} qtspellchecking ${ENCHANT_LIBRARIES} Qt5::Core)
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${QT5EXTRA_PLUGIN_DIR})

