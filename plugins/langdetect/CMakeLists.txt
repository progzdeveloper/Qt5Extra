cmake_path(SET QT5EXTRA_LANGDETECT_ROOT "${QT5EXTRA_PLUGINS_ROOT}/langdetect")
message(STATUS "QT5EXTRA_LANGDETECT_ROOT=${QT5EXTRA_LANGDETECT_ROOT}")

add_subdirectory(cld2plugin)
#add_subdirectory(langdetectcxxplugin)
if (WIN32)
    add_subdirectory(win32ldsplugin)
endif (WIN32)
