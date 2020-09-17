#######################################################
### Executable                                      ###
#######################################################
add_executable(bibexplorer bibexplorer.cpp)
target_link_libraries(bibexplorer PRIVATE bibliometrics)

#######################################################
### Installer                                       ###
#######################################################
if(BUILD_INSTALLER)
    # Install the executable only
    install(TARGETS bibexplorer
            EXPORT BibExplorerTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()
