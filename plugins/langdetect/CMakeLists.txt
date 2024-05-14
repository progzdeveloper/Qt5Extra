cmake_path(SET QT5EXTRA_LANGDETECT_ROOT "${QT5EXTRA_PLUGINS_ROOT}/langdetect")
message(STATUS "QT5EXTRA_LANGDETECT_ROOT=${QT5EXTRA_LANGDETECT_ROOT}")

#add_subdirectory(cld2)
#add_subdirectory(langdetectcxx)
if (WIN32)
    add_subdirectory(win32lds)
endif (WIN32)
