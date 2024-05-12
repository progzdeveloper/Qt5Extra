cmake_path(SET QT5EXTRA_PLUGINS_ROOT "${QT5EXTRA_ROOT}/plugins")
message(STATUS "QT5EXTRA_PLUGINS_ROOT=${QT5EXTRA_PLUGINS_ROOT}")

add_subdirectory(modelexporters)
add_subdirectory(spellcheckers)

