cmake_path(SET QT5EXTRA_SPELLCHECKERS_ROOT "${QT5EXTRA_PLUGINS_ROOT}/spellcheckers")
message(STATUS "QT5EXTRA_SPELLCHECKERS_ROOT=${QT5EXTRA_SPELLCHECKERS_ROOT}")

if(UNIX AND NOT APPLE) # Linux
    add_subdirectory(hunspellbackend)
    add_subdirectory(enchantbackend)
    #add_subdirectory(aspellbackend)
endif()

if(WIN32) # Windows
	add_subdirectory(win32spellbackend)
endif()

if(APPLE) # MacOS
    add_subdirectory(nsspellbackend)
endif()
