macro(debug_pkg_config pkg)
    foreach (prefix ${pkg} ${pkg}_STATIC)
        foreach (opt "INCLUDE_DIRS" "CFLAGS" "CFLAGS_OTHER" "LIBRARIES" "LINK_LIBRARIES" "LIBRARY_DIRS" "LDFLAGS" "LDFLAGS_OTHER")
            message(STATUS "${prefix}_${opt}=${${prefix}_${opt}}")
        endforeach ()
    endforeach ()
endmacro()


function(tcenv_find_pkgconfig pkgname)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "DEBUG" "" "")

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(${pkgname} REQUIRED ${pkgname})

    if (ARG_DEBUG)
        debug_pkg_config(${pkgname})
    endif ()
    if (${${pkgname}_FOUND})
        add_library(${pkgname}::${pkgname} INTERFACE IMPORTED)
        add_library(${pkgname}::static INTERFACE IMPORTED)
        add_library(${pkgname}::static-nodep INTERFACE IMPORTED)

        if(${pkgname}_INCLUDE_DIRS)
          set_property(TARGET ${pkgname}::${pkgname} PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${${pkgname}_INCLUDE_DIRS}")
          set_property(TARGET ${pkgname}::static PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${${pkgname}_INCLUDE_DIRS}")
          set_property(TARGET ${pkgname}::static-nodep PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${${pkgname}_INCLUDE_DIRS}")
        endif()
        if(${pkgname}_CFLAGS_OTHER)
          set_property(TARGET ${pkgname}::${pkgname} PROPERTY INTERFACE_COMPILE_OPTIONS "${${pkgname}_CFLAGS_OTHER}")
        endif()
        if(${pkgname}_STATIC_CFLAGS_OTHER)
          set_property(TARGET ${pkgname}::static-nodep PROPERTY INTERFACE_COMPILE_OPTIONS "${${pkgname}_STATIC_CFLAGS_OTHER}")
          set_property(TARGET ${pkgname}::static PROPERTY INTERFACE_COMPILE_OPTIONS "${${pkgname}_STATIC_CFLAGS_OTHER}")
        endif()
        if(${pkgname}_LIBRARY_DIRS)
          set_property(TARGET ${pkgname}::${pkgname} PROPERTY INTERFACE_LINK_DIRECTORIES "${${pkgname}_LIBRARY_DIRS}")
          set_property(TARGET ${pkgname}::static PROPERTY INTERFACE_LINK_DIRECTORIES "${${pkgname}_LIBRARY_DIRS}")
          set_property(TARGET ${pkgname}::static-nodep PROPERTY INTERFACE_LINK_DIRECTORIES "${${pkgname}_LIBRARY_DIRS}")
        endif()
        if(${pkgname}_LIBRARIES)
          set_property(TARGET ${pkgname}::${pkgname} PROPERTY INTERFACE_LINK_LIBRARIES "${${pkgname}_LIBRARIES}")
          set_property(TARGET ${pkgname}::static-nodep PROPERTY INTERFACE_LINK_LIBRARIES "${${pkgname}_LIBRARIES}")
        endif()
        if(${pkgname}_STATIC_LIBRARIES)
          set_property(TARGET ${pkgname}::static PROPERTY INTERFACE_LINK_LIBRARIES "${${pkgname}_STATIC_LIBRARIES}")
        endif()
    endif ()
endfunction()
